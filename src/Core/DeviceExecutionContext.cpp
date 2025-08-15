// standard headers
#include <functional>
#include <vector>
#include <mutex>

// external headers
#include "Volk/volk.h"

// project headers
#include "Lettuce/Core/DeviceExecutionContext.hpp"

using namespace Lettuce::Core;

std::vector<std::unique_lock<std::mutex>> DeviceExecutionContext::lockAllCmdPools()
{
    std::vector<std::unique_lock<std::mutex>> locks;
    locks.reserve(mtxs.size());

    for (auto &m : mtxs)
        locks.emplace_back(m, std::defer_lock);

    // lock all mutexes here
    std::lock(locks.begin(), locks.end());
    return locks;
}

void DeviceExecutionContext::workThreadFunc(uint32_t threadIndex)
{
    while (!threadShouldExit)
    {
        // wait for the main thread to signal work
        syncBarrier.arrive_and_wait();

        // for release(), check if thread should exit
        if (threadShouldExit)
            break;

        uint32_t taskIndex = 0;
        // execute tasks
        while (true)
        {
            std::function<void(uint32_t, VkCommandBuffer)> task;

            {
                // lock access to tasks queue to get one task
                std::lock_guard<std::mutex> lock(tasksAccessMutex);
                // if there are no tasks, break the loop to signal that this thread is ready
                if (tasks.empty())
                    break;

                // get the task from the queue
                task = std::move(tasks.front());
                tasks.pop();
            }

            uint32_t maxTasks = cmds.size() / cmdPools.size();
            // Each task contains information about recording commands
            // Synchronization is handled by this thread
            // this lock ensures that one pool is accessed by only one thread at a time
            std::lock_guard<std::mutex> lock(cmdPoolAccessMutexes[threadIndex]);
            auto cmd = cmds[(threadIndex * maxTasks) + taskIndex];
            task(threadIndex, cmd);
            taskIndex++;
        }

        // signal that this thread is ready and work is done
        syncBarrier.arrive_and_wait();
    }
}

void DeviceExecutionContext::setupThreads(const DeviceExecutionContextCreateInfo &createInfo)
{
    for (uint32_t i = 0; i < createInfo.threadCount; ++i)
    {
        workThreads.emplace_back(&DeviceExecutionContext::workThreadFunc, this, i);
    }
}

void DeviceExecutionContext::setupSynchronizationPrimitives(const DeviceExecutionContextCreateInfo &createInfo)
{
    threadShouldExit = false;
    barrier(createInfo.threadCount + 1); // worker threads + main thread
    cmdPoolAccessMutexes.resize(createInfo.threadCount);
}

void DeviceExecutionContext::setupCommandPools(const DeviceExecutionContextCreateInfo &createInfo)
{
    auto locks = lockAllCmdPools();
    uint32_t cmdCountPerThread = createInfo.maxTasks;

    cmdPools.resize(createInfo.threadCount);
    cmds.resize(createInfo.threadCount * cmdCountPerThread);

    for (uint32_t i = 0; i < createInfo.threadCount; ++i)
    {
        VkCommandPoolCreateInfo cmdPoolCI = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = 0, // TODO: get correct queue family index
        };
        checkResult(vkCreateCommandPool(m_device, &cmdPoolCI, nullptr, &cmdPools[i]));

        VkCommandBufferAllocateInfo cmdAllocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = cmdPools[i],
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = cmdCountPerThread,
        };
        checkResult(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &cmds[i * cmdCountPerThread]));
    }
}

DeviceExecutionContext::DeviceExecutionContext(VkDevice device, const DeviceExecutorCreateInfo &createInfo, LettuceResult& result)
{
    m_device = device;
    setupSynchronizationPrimitives(createInfo);
    setupCommandPools(createInfo);
    setupThreads(createInfo);
}

void DeviceExecutionContext::Release()
{
    // declare threads should exit
    {
        std::lock_guard<std::mutex> lock(tasksAccessMutex);
        threadShouldExit = true;
    }
    // wait for all threads to reach the barrier & join them
    syncBarrier.arrive_and_wait();
    for (auto &thread : workThreads)
    {
        if (thread.joinable())
            thread.join();
    }

    uint32_t maxTasks = cmds.size() / cmdPools.size();
    for (int i = 0; i < cmdPools.size(); ++i)
    {
        vkFreeCommandBuffers(m_device, cmdPools[i], maxTasks, cmds.data() + (i * maxTasks));
        vkDestroyCommandPool(m_device, cmdPools[i], nullptr);
    }

    cmds.clear();
    cmds.shrink_to_fit();
    cmdPools.clear();
    cmdPools.shrink_to_fit();
}

void DeviceExecutionContext::Prepare(const std::vector<std::vector<CommandList>> &cmds)
{
}

void DeviceExecutionContext::Record()
{
    // signal thread to start recording
    syncBarrier.arrive_and_wait();

    // wait for threads to finish recording
    syncBarrier.arrive_and_wait();
}

void DeviceExecutionContext::Execute()
{
}