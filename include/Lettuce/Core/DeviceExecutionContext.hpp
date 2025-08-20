/*
Created by @PieroCastillo on 2025-08-11
*/
#ifndef LETTUCE_CORE_DEVICE_EXECUTION_CONTEXT_HPP
#define LETTUCE_CORE_DEVICE_EXECUTION_CONTEXT_HPP

// standard headers
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <atomic>
#include <queue>
#include <functional>
#include <barrier>

// project headers
#include "Common.hpp"

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
        std::vector<VkCommandsPool> cmdPools;
        std::vector<VkCommandBuffer> cmds;
        std::vector<std::mutex> cmdPoolAccessMutexes;

        std::queue<VkCommandsBuffer> readyCmds;

        std::queue<std::function<void(uint32_t, VkCommandBuffer)>> tasks;
        std::mutex tasksAccessMutex;

        std::atomic<bool> threadShouldExit;
        std::barrier<> syncBarrier;

        std::mutex queueSubmitMutex;

        // called one time
        inline std::vector<std::unique_lock<std::mutex>> lockAllCmdPools();
        inline void workThreadFunc(uint32_t threadIndex);
        inline void setupThreads(const DeviceExecutionContextCreateInfo &createInfo);
        inline void setupSynchronizationPrimitives(const DeviceExecutionContextCreateInfo &createInfo);
        inline void setupCommandPools(const DeviceExecutionContextCreateInfo &createInfo);

    public:
        VkDevice m_device;
        VkQueue m_graphicsQueue;
        VkQueue m_computeQueue;
        VkQueue m_transferQueue;

        LettuceResult Create(VkDevice device, const DeviceExecutorCreateInfo &createInfo);
        void Release();

        void Prepare(const std::vector<std::vector<CommandList>> &cmds);
        void Record();
        void Execute();
    };
}
#endif // LETTUCE_CORE_DEVICE_EXECUTION_CONTEXT_HPP