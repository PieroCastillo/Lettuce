/*
Created by @PieroCastillo on 2025-11-4
*/
#ifndef LETTUCE_CORE_ALLOCATORS_LINEAR_GPU_RESOURCE_HPP
#define LETTUCE_CORE_ALLOCATORS_LINEAR_GPU_RESOURCE_HPP

// standard headers
#include <memory_resource>

// project headers
#include "../common.hpp"

namespace Lettuce::Core::Allocators
{
    struct GPUMonotonicBufferResourceCreateInfo
    {
        MemoryAccess memoryAccess;
        AllocatorUsage memoryUsage;
        uint32_t maxSize;
    };

    class GPUMonotonicBufferResource : public std::pmr::memory_resource
    {
        VkDevice m_device;
        VkBuffer m_buffer;
        VkDeviceMemory m_memory;
        uint32_t m_size;
        MemoryAccess m_access;
        void* m_mappedData;
        uint32_t m_offset;
    public:
        GPUMonotonicBufferResource(const IDevice& device, const GPUMonotonicBufferResourceCreateInfo& createInfo);
        ~GPUMonotonicBufferResource();

        void Reset();
        
        void* do_allocate(size_t bytes, size_t alignment) override;
        void do_deallocate(void*, size_t, size_t) override;
        bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override;
    };
};

#endif // LETTUCE_CORE_ALLOCATORS_LINEAR_GPU_RESOURCE_HPP