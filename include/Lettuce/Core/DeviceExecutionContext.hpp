/*
Created by @PieroCastillo on 2025-08-11
*/
#ifndef LETTUCE_CORE_DEVICE_EXECUTION_CONTEXT_HPP
#define LETTUCE_CORE_DEVICE_EXECUTION_CONTEXT_HPP

// standard headers
#include <thread>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <vector>
#include <atomic>
#include <queue>
#include <functional>
#include <barrier>

// project headers
#include "common.hpp"
#include "CommandList.hpp"

namespace Lettuce::Core
{
    struct DeviceExecutionContextCreateInfo
    {
        uint32_t threadCount;
        uint32_t maxTasks;
    };

    /*
        Responsabilities:
        - Create and manage recording threads
        - Submit command buffers to the device
        - Handle synchronization between threads
        - Manage synchronization primitives for threads
        - Manage command pools and command buffers
        - Manage Vulkan synchronization objects
    */
    class DeviceExecutionContext
    {
    private:
        std::vector<std::thread> workThreads;
        std::vector<VkCommandPool> cmdPools;
        std::vector<VkCommandBuffer> cmds;
        std::vector<std::mutex> cmdPoolAccessMutexes;

        std::queue<VkCommandBuffer> readyCmds;

        std::queue<std::function<void(uint32_t, VkCommandBuffer)>> tasks;
        std::mutex tasksAccessMutex;

        std::atomic<bool> threadShouldExit;

        std::mutex queueSubmitMutex;

        // called one time
        void workThreadFunc(uint32_t threadIndex, std::barrier<>& syncBarrier);
        void setupThreads(const DeviceExecutionContextCreateInfo& createInfo);
        void setupSynchronizationPrimitives(const DeviceExecutionContextCreateInfo& createInfo);
        void setupCommandPools(const DeviceExecutionContextCreateInfo& createInfo);

    public:
        VkDevice m_device;
        VkQueue m_graphicsQueue;
        VkQueue m_computeQueue;
        VkQueue m_transferQueue;

        void Create(const std::weak_ptr<IDevice>& device, const DeviceExecutionContextCreateInfo& createInfo);
        void Release();

        void Prepare(const std::vector<std::vector<CommandList>>& cmds);
        void Record();
        void Execute();
    };
}
#endif // LETTUCE_CORE_DEVICE_EXECUTION_CONTEXT_HPP