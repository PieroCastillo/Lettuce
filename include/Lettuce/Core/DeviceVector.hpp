/*
Created by @PieroCastillo on 2025-10-22
*/
#ifndef LETTUCE_CORE_DEVICE_VECTOR_HPP
#define LETTUCE_CORE_DEVICE_VECTOR_HPP

// standard headers
#include <optional>
#include <memory>
#include <vector>
#include <string>

// project headers
#include "common.hpp"

namespace Lettuce::Core
{
    // struct DeviceVectorCreateInfo
    // {
    //     uint32_t maxCount;
    //     std::shared_ptr<Allocators::BlockAllocator> allocator;
    // };

    // class DeviceVectorBase
    // {
    //     VkDevice m_device;
    //     BufferAllocation m_allocation;
    //     std::shared_ptr<Allocators::BlockAllocator> m_allocator;
    //     uint32_t m_elementSize;
    //     uint32_t m_maxSize;

    //     uint32_t m_offset;
    // public:
    //     void Create(const IDevice& device, const DeviceVectorCreateInfo& createInfo, uint32_t elementSize);
    //     void Release();

    //     void PushRange(void* src, uint32_t count);
    //     void Flush();
    //     void Reset();
    // };

    // template<typename T>
    // class DeviceVector
    // {    
    //     DeviceVectorBase base;
    // public:
    //     void Create(const IDevice& device, const DeviceVectorCreateInfo& createInfo);
    //     void Release();

    //     void Push(const T& element);
    //     void Flush();
    //     void Reset();
    // };
};

#include "DeviceVector.inl"
#endif // LETTUCE_CORE_DEVICE_VECTOR_HPP