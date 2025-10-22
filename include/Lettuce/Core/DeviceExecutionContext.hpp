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
#include <latch>

// project headers
#include "common.hpp"
#include "CommandList.hpp"

namespace Lettuce::Core
{
    class DeviceThreadPool
    {
    public:
        void Create(uint32_t threadCount);
        void Release();

        template<class F>
        void Enqueue(F&& f) {
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                tasks.emplace(std::forward<F>(f));
            }
            cv.notify_one();
        }

    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
        std::mutex queueMutex;
        std::condition_variable cv;
        bool stop;
    };

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
        DeviceThreadPool threadPool;
        std::vector<VkCommandPool> cmdPools;
        std::vector<VkCommandBuffer> cmds;
        std::vector<std::mutex> cmdPoolAccessMutexes;

        // called one time
        void setupSynchronizationPrimitives(const DeviceExecutionContextCreateInfo& createInfo);
        void setupCommandPools(const DeviceExecutionContextCreateInfo& createInfo);

    public:
        VkDevice m_device;
        VkQueue m_graphicsQueue;
        VkQueue m_computeQueue;
        VkQueue m_transferQueue;

        void Create(const IDevice& device, const DeviceExecutionContextCreateInfo& createInfo);
        void Release();

        void Prepare(const std::vector<std::vector<CommandList>>& cmds);
        void Record();
        void Execute();
    };
}
#endif // LETTUCE_CORE_DEVICE_EXECUTION_CONTEXT_HPP