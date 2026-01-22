// standard headers
#include <memory>
#include <memory_resource>
#include <span>
#include <vector>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Rendering/api.hpp"
#include "Lettuce/Core/api.hpp"

using namespace Lettuce::Rendering;
using namespace Lettuce::Core;

void AsyncRecorder::Create(AsyncRecorderDesc desc)
{
    device = desc.device;
    CommandAllocatorDesc allocDesc = {
        .queueType = QueueType::Graphics,
    };
    allocators.resize(desc.threadCount);

    // reserve for 16 levels, are you gonna need more?
    cmdLevels.reserve(16);
    stop = false;
    currentLevel = 0;

    for (int threadID = 0; threadID < desc.threadCount; ++threadID)
    {
        allocators[threadID] = device.CreateCommandAllocator(allocDesc);

        threads.emplace_back([this, threadID]
            {
                auto alloc = allocators[threadID];

                while (!stop.load(std::memory_order_acquire))
                {
                    // get task
                    Task task;
                    {
                        std::unique_lock queueLock(tasksMutex);

                        taskCV.wait(queueLock, [&] { return stop.load(std::memory_order_relaxed) || !taskQueue.empty(); });
                        if (stop && taskQueue.empty())
                            continue;
                        task = std::move(taskQueue.front());
                        taskQueue.pop();
                    }
                    auto cmd = device.AllocateCommandBuffer(allocators[threadID]);
                    task.recordFunc(cmd, task.userData);

                    {
                        std::lock_guard lock(cmdsMutex);
                        cmdLevels[task.taskLevel].push_back(cmd);
                    }

                    auto oldPend = pendingTasks.fetch_sub(1, std::memory_order_acquire);
                    if (oldPend == 1)
                    {
                        pendingTasks.notify_all();
                    }
                }
            });
    }
}

void AsyncRecorder::Destroy()
{
    {
        std::lock_guard lock(tasksMutex);
        stop.store(true, std::memory_order_release);
    }
    taskCV.notify_all();

    for (int threadID = 0; threadID < threads.size(); ++threadID)
    {
        device.Destroy(allocators[threadID]);
        threads[threadID].join();
    }
    cmdLevels.clear();
}

void AsyncRecorder::Reset()
{
    std::lock_guard lock(cmdsMutex);
    for (int id = 0; id < threads.size(); ++id)
    {
        device.Reset(allocators[id]);
    }
    cmdLevels.clear();
    cmdLevels.push_back({});
    currentLevel.store(0, std::memory_order_release);
}

void AsyncRecorder::RecordAsync(std::any userData, std::move_only_function<void(CommandBuffer, std::any)>&& record)
{
    // add one task to the queue & inc atomic counter
    pendingTasks.fetch_add(1, std::memory_order_acquire);
    std::lock_guard lockQueue(tasksMutex);

    auto task = Task{
        .taskLevel = currentLevel.load(std::memory_order_acquire),
        .userData = userData,
        .recordFunc = std::move(record),
    };

    taskQueue.push(std::move(task));
    taskCV.notify_one();
}

void AsyncRecorder::Barrier()
{
    // mark that the next cmds go to another level
    // dont stop current buffer recording
    std::lock_guard lock(cmdsMutex);
    cmdLevels.push_back({});
    currentLevel.fetch_add(1, std::memory_order_release);
}

void AsyncRecorder::Submit(std::optional<Swapchain> optSwapchain)
{
    // wait until remaining tasks == 0
    while (pendingTasks.load(std::memory_order_acquire) > 0)
    {
        pendingTasks.wait(pendingTasks.load(std::memory_order_relaxed), std::memory_order_acquire);
    }

    // create buffer on stack (because we need to be fast)
    std::byte stackBuffer[4096];
    std::pmr::monotonic_buffer_resource stackRes(stackBuffer, sizeof(stackBuffer));
    std::pmr::vector<std::span<CommandBuffer>> cmdsVec(&stackRes);

    for (auto& levelCmds : cmdLevels)
    {
        cmdsVec.push_back(std::span<CommandBuffer>(levelCmds));
    }

    CommandBufferSubmitDesc submitDesc = {
        .queueType = QueueType::Graphics,
        .commandBuffers = std::span(cmdsVec),
        .presentSwapchain = optSwapchain,
    };
    device.Submit(submitDesc);
}