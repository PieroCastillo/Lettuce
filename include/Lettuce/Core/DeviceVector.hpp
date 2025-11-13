/*
Created by @PieroCastillo on 2025-10-22
*/
#ifndef LETTUCE_CORE_DEVICE_VECTOR_HPP
#define LETTUCE_CORE_DEVICE_VECTOR_HPP

// standard headers
#include <optional>
#include <memory>
#include <span>
#include <string>

// project headers
#include "common.hpp"
#include "Allocators/IGPUMemoryResource.hpp"

namespace Lettuce::Core
{
    struct DeviceVectorCreateInfo
    {
        uint32_t maxCount;
        std::shared_ptr<Allocators::IGPUMemoryResource> allocator;
    };

    class DeviceVectorBase
    {
        VkDevice m_device;
        BufferAllocation m_allocation;
        std::shared_ptr<Allocators::IGPUMemoryResource> m_allocator;
        uint64_t m_elementSize;
        uint64_t m_maxSize;

        uint64_t m_offset;
    public:
        void Create(const IDevice& device, const DeviceVectorCreateInfo& createInfo, uint32_t elementSize);
        void Release();

        void PushRange(void* src, uint32_t count);
        void Flush();
        void Reset();
    };

    template<typename T>
    class DeviceVector
    {    
        DeviceVectorBase base;
    public:
        void Create(const IDevice& device, const DeviceVectorCreateInfo& createInfo);
        void Release();

        void Push(const T& element);
        void CopyFrom(const std::span<T>& dst);
        void Flush();
        void Reset();
    };
};

#include "DeviceVector.inl"
#endif // LETTUCE_CORE_DEVICE_VECTOR_HPP