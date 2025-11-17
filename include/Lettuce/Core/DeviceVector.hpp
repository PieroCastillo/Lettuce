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
#include <typeindex>

// project headers
#include "common.hpp"
#include "Allocators/IGPUMemoryResource.hpp"

namespace Lettuce::Core
{
    // forward declarations
    class Device;

    struct DeviceVectorCreateInfo
    {
        uint32_t maxCount;
        std::shared_ptr<Allocators::IGPUMemoryResource> allocator;
    };

    class DeviceVectorBase
    {
    private:
        friend class Device;
        template<typename> friend class DeviceVector;
        VkDevice m_device;
        BufferAllocation m_allocation;
        std::shared_ptr<Allocators::IGPUMemoryResource> m_allocator;
        std::type_index typeIdx;
        uint64_t m_elementSize;
        uint64_t m_maxSize;
        uint64_t m_address;

        uint64_t m_offset;
    public:
        DeviceVectorBase() : typeIdx(typeid(void)) {}
        void Create(const IDevice& device, const DeviceVectorCreateInfo& createInfo, uint32_t elementSize);
        void Release();

        void PushRange(void* src, uint32_t count);
        void Flush();
        void Reset();
        
        BufferHandle GetHandle();
    };

    template<typename T>
    class DeviceVector
    {    
        friend class Device;
        DeviceVectorBase base;
    public:
        void Create(const IDevice& device, const DeviceVectorCreateInfo& createInfo);
        void Release();

        void Push(const T& element);
        void CopyFrom(const std::span<T>& dst);
        void Flush();
        void Reset();

        BufferHandle GetHandle();
    };
};

#include "DeviceVector.inl"
#endif // LETTUCE_CORE_DEVICE_VECTOR_HPP