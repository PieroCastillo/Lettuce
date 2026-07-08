/*
Created by @PieroCastillo on 2026-06-01
*/
#ifndef LETTUCE_UTILS_GPU_STORAGE_VECTOR
#define LETTUCE_UTILS_GPU_STORAGE_VECTOR

// standard headers
#include <utility>

// project headers
#include "../Core/api.hpp"
#include "Common.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Utils
{
    template<GpuElement T>
    class GpuStorageVector
    {
    private:
        Device* m_device;
        MemoryView m_memView;
        MemoryViewInfo m_info;
        uint32_t m_capacity;
    public:
        GpuStorageVector() noexcept = default;

        /// @brief Constructor
        /// @param device Refrenced Device
        /// @param capacity Capacity, must be greater than 0
        explicit GpuStorageVector(Device& device, uint32_t capacity)
            : m_device(&device), m_capacity(capacity)
        {
            DebugAssert(capacity > 0, "Capacity MUST be greater than 0");

            m_memView = device.CreateMemoryView({ sizeof(T) * capacity, false });
            m_info = device.GetMemoryViewInfo(m_memView);
        };

        /// @brief Copy Constructor. Deleted.
        GpuStorageVector(const GpuStorageVector&) = delete;

        /// @brief Move Constructor.
        GpuStorageVector(GpuStorageVector&& other) noexcept :
            m_device(other.m_device),
            m_memView(std::exchange(other.m_memView, {})),
            m_info(std::exchange(other.m_info, {})),
            m_capacity(std::exchange(other.m_capacity, 0))
        {
        }

        /// @brief Copy assignment operator. Deleted.
        auto operator=(const GpuStorageVector&) -> GpuStorageVector & = delete;

        /// @brief Move assignment operator. Deleted.
        auto operator=(GpuStorageVector&&) -> GpuStorageVector & = delete;

        /// @brief Destructor.
        ~GpuStorageVector()
        {
            if (m_memView.generation == 0)
                return;

            m_device->Destroy(m_memView);
            m_memView = {};
            m_info = {};
            m_capacity = 0;
        }

        auto byteSize() const noexcept -> uint32_t { return m_info.size; }
        auto size() const noexcept -> uint32_t { return m_size; }
        auto capacity() const noexcept -> uint32_t { return m_capacity; }
        auto empty() const noexcept -> bool { return m_size == 0; }

        auto deviceData() const noexcept -> DeviceAddress { return m_info.gpuAddress; }
    };
};
#endif // LETTUCE_UTILS_GPU_STORAGE_VECTOR