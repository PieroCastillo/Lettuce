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
    CommandAllocatorDesc allocDesc = {
        .queueType = QueueType::Graphics,
    };
    allocators.resize(desc.threadCount);

    // reserve for 48 Command buffers, are you gonna need more?
    cmds.reserve(48);
    counts.reserve(48);

    for (int threadID = 0; threadID < desc.threadCount; ++threadID)
    {
        allocators[threadID] = device.CreateCommandAllocator(allocDesc);

        threads.emplace_back([this, threadID]
            {
                auto alloc = allocators[threadID];

                while (stop.load(std::memory_order_acquire))
                {
                    // get task
                    Task task;
                    {
                        std::unique_lock queueLock(tasksMutex);

                        taskCV.wait(queueLock, [&] { return stop.load(std::memory_order_relaxed) || !taskQueue.empty(); });
                        if (stop && taskQueue.empty())
                            return;
                        task = std::move(taskQueue.front());
                        taskQueue.pop();
                    }
                    auto cmd = device.AllocateCommandBuffer(allocators[threadID]);
                    task.recordFunc(cmd, task.userData);

                    {
                        std::lock_guard lock(cmdsMutex);
                        cmds.push_back(cmd);
                        ++counts.back();
                    }

                    if (pendingTasks.fetch_sub(1, std::memory_order_acq_rel) == 1)
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
    cmds.clear();
    counts.clear();
}

void AsyncRecorder::RecordAsync(std::any userData, std::move_only_function<void(CommandBuffer, std::any)>&& record)
{
    // add one task to the queue & inc atomic counter
    pendingTasks.fetch_add(1, std::memory_order_acquire);
    std::lock_guard lockQueue(tasksMutex);

    auto task = Task{
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
    counts.push_back(0);
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

    uint32_t offset = 0;
    for (auto count : counts)
    {
        cmdsVec.emplace_back(cmds.data() + offset, count);
        offset += count;
    }

    CommandBufferSubmitDesc submitDesc = {
        .queueType = QueueType::Graphics,
        .commandBuffers = std::span(cmdsVec),
        .presentSwapchain = optSwapchain,
    };
    device.Submit(submitDesc);
}