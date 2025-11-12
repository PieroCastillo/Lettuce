/*
Created by @PieroCastillo on 2025-11-4
*/
#ifndef LETTUCE_CORE_ALLOCATORS_LINEAR_IMAGE_ALLOCATOR_HPP
#define LETTUCE_CORE_ALLOCATORS_LINEAR_IMAGE_ALLOCATOR_HPP

// project headers
#include "../common.hpp"
#include "IGPUMemoryResource.hpp"

namespace Lettuce::Core::Allocators
{
    struct LinearImageAllocCreateInfo
    {
        MemoryAccess memoryAccess;
        uint32_t maxSize;
    };

    class LinearImageAlloc : public IGPUMemoryResource
    {
        VkDevice m_device;
        VkDeviceMemory m_memory;
        uint32_t m_size;
        uint32_t m_offset;
        MemoryAccess m_access;

        std::vector<VkImage> m_images;
    public:
        void Create(const IDevice& device, const LinearImageAllocCreateInfo& createInfo);
        void Release();
        void Reset();

        BufferAllocation do_balloc(uint32_t size, VkBufferUsageFlags usage) override;
        ImageAllocation do_ialloc(VkFormat format, VkExtent2D extent, uint32_t mipLevels, uint32_t arrayLayers, VkImageUsageFlags usage) override;
        void do_bdeallocate(const BufferAllocation& allocation) override;
        void do_ideallocate(const ImageAllocation& allocation) override;

        MemoryAccess GetMemoryAccess() override;
    };
};

#endif // LETTUCE_CORE_ALLOCATORS_LINEAR_IMAGE_ALLOCATOR_HPP