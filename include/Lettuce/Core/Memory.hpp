/*
Created by @PieroCastillo on 2025-07-27
*/
#ifndef LETTUCE_CORE_MEMORY_HPP
#define LETTUCE_CORE_MEMORY_HPP 

// project headers
#include "Common.hpp"

namespace Lettuce::Core
{
    enum class MemoryAccess
    {
        FastGPUReadWrite, // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        FastCPUWrite, // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
        FastGPUWriteCPURead, // VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    };

    struct MemoryCreateInfo
    {
        uint32_t size; // in bytes
        MemoryAccess access;
    };

    class Memory
    {
    private:
    
    public:
        VkDevice m_device;
        VkDeviceMemory m_memory;

        Memory(VkDevice device, const MemoryCreateInfo& createInfo);
        void Release();
    };
}
#endif // LETTUCE_CORE_MEMORY_HPP