/*
Created by @PieroCastillo on 2025-08-11
*/
#ifndef LETTUCE_CORE_DEVICE_EXECUTOR_HPP
#define LETTUCE_CORE_DEVICE_EXECUTOR_HPP 

namespace Lettuce::Core
{
    struct DeviceExecutorCreateInfo
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
    class DeviceExecutor
    {
    private:
    
    public:
        VkDevice m_device;

        DeviceExecutor(VkDevice device, const DeviceExecutorCreateInfo& createInfo);
        void Release();
    };
}
#endif // LETTUCE_CORE_DEVICE_EXECUTOR_HPP