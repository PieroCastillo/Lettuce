/*
Created by @PieroCastillo on 2025-11-4
*/
#ifndef LETTUCE_CORE_ALLOCATORS_LINEAR_BUFFER_SUBALLOCATOR_HPP
#define LETTUCE_CORE_ALLOCATORS_LINEAR_BUFFER_SUBALLOCATOR_HPP

// project headers
#include "../common.hpp"
#include "IGPUMemoryResource.hpp"

namespace Lettuce::Core::Allocators
{
    struct LinearBufferSubAllocCreateInfo
    {
        MemoryAccess memoryAccess;
        AllocatorUsage memoryUsage;
        uint32_t maxSize;
    };

    class LinearBufferSubAlloc : public  IGPUMemoryResource
    {
        VkDevice m_device;
        VkBuffer m_buffer;
        VkDeviceMemory m_memory;
        uint32_t m_size;
        MemoryAccess m_access;
        void* m_mappedData;
        uint32_t m_offset;
    public:
        void Create(const IDevice& device, const LinearBufferSubAllocCreateInfo& createInfo);
        void Release();
        void Reset();

        BufferAllocation do_balloc(uint32_t size, VkBufferUsageFlags usage) override;
        ImageAllocation do_ialloc(VkFormat format, VkExtent2D extent, uint32_t mipLevels, uint32_t arrayLayers, VkImageUsageFlags usage) override;
        void do_bdeallocate(const BufferAllocation& allocation) override;
        void do_ideallocate(const ImageAllocation& allocation) override;

        MemoryAccess GetMemoryAccess() override;
    };
};

#endif // LETTUCE_CORE_ALLOCATORS_LINEAR_BUFFER_SUBALLOCATOR_HPP