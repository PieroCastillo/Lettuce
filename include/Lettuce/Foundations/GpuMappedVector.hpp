/*
Created by @PieroCastillo on 2026-06-01
*/
#ifndef LETTUCE_UTILS_GPU_MAPPED_VECTOR
#define LETTUCE_UTILS_GPU_MAPPED_VECTOR

// standard headers
#include <utility>

// project headers
#include "../Core/api.hpp"
#include "../Core/Common.hpp"
#include "../helper.hpp"
#include "Common.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Foundations
{
    template<GpuElement T>
    class GpuMappedVector
    {
    private:
        Device* m_device;
        MemoryView m_memView;
        MemoryViewInfo m_info;
        uint32_t m_capacity;
        uint32_t m_size;
    public:
        using value_type = T;
        using iterator = T*;
        using const_iterator = const T*;

        GpuMappedVector() noexcept = default;

        /// @brief Constructor
        /// @param device Referred Device
        /// @param capacity Capacity, must be greater than 0
        explicit GpuMappedVector(Device& device, uint32_t capacity)
            : m_device(&device), m_capacity(capacity), m_size(0)
        {
            DebugAssert(capacity > 0, "Capacity MUST be greater than 0");

            m_memView = device.CreateMemoryView({ sizeof(T) * capacity, true });
            m_info = device.GetMemoryViewInfo(m_memView);
        }

        /// @brief Copy Constructor. Deleted.
        GpuMappedVector(const GpuMappedVector&) = delete;

        /// @brief Move Constructor.
        GpuMappedVector(GpuMappedVector&& other) noexcept :
            m_device(other.m_device),
            m_memView(std::exchange(other.m_memView, {})),
            m_info(std::exchange(other.m_info, {})),
            m_capacity(std::exchange(other.m_capacity, 0)),
            m_size(std::exchange(other.m_size, 0))
        {
        }

        /// @brief Copy assignment operator. Deleted.
        auto operator=(const GpuMappedVector&) -> GpuMappedVector & = delete;

        /// @brief Move assignment operator
        auto operator=(GpuMappedVector&& other) noexcept -> GpuMappedVector&
        {
            DebugAssert(m_memView.generation == 0, "GpuMappedVector cannot be reassigned");

            if (this == &other)
                return *this;

            m_device = std::exchange(other.m_device, nullptr);
            m_memView = std::exchange(other.m_memView, {});
            m_info = std::exchange(other.m_info, {});
            m_capacity = std::exchange(other.m_capacity, 0);
            m_size = std::exchange(other.m_size, 0);

            return *this;
        }

        /// @brief Destructor.
        ~GpuMappedVector()
        {
            if (m_memView.generation == 0)
                return;

            m_device->Destroy(m_memView);
            m_memView = {};
            m_info = {};
            m_capacity = 0;
            m_size = 0;
        }

        auto byteSize() const noexcept -> uint32_t { return m_info.size; }
        auto size() const noexcept -> uint32_t { return m_size; }
        auto capacity() const noexcept -> uint32_t { return m_capacity; }
        auto empty() const noexcept -> bool { return m_size == 0; }

        auto data() noexcept -> T* { return (T*)m_info.cpuAddress; }
        auto data() const noexcept -> const T* { return (T*)m_info.cpuAddress; }

        auto begin() noexcept -> iterator { return data(); }
        auto end() noexcept -> iterator { return data() + m_size; }
        auto begin() const noexcept -> const_iterator { return data(); }
        auto end() const noexcept -> const_iterator { return data() + m_size; }

        auto operator[](uint32_t index) noexcept -> T& { return ((T*)m_info.cpuAddress)[index]; }
        auto operator[](uint32_t index) const noexcept -> const T& { return ((T*)m_info.cpuAddress)[index]; }

        auto first() noexcept -> T& { return ((T*)m_info.cpuAddress)[0]; }
        auto last() noexcept -> T& { return ((T*)m_info.cpuAddress)[m_size - 1]; }
        auto first() const noexcept -> const T& { return ((T*)m_info.cpuAddress)[0]; }
        auto last() const noexcept -> const T& { return ((T*)m_info.cpuAddress)[m_size - 1]; }

        auto push_back(const T& value) -> uint32_t
        {
            DebugAssert(m_size < m_capacity, "size MUST be less than capacity to push a value.");

            const uint32_t index = m_size;
            data()[index] = value;
            ++m_size;

            return index;
        }

        auto append(std::span<const T> values) -> std::pair<uint32_t, uint32_t>
        {
            DebugAssert(m_size + values.size() <= m_capacity, "total size MUST be less or equal than capacity.");

            const uint32_t firstIndex = m_size;
            const uint32_t count = static_cast<uint32_t>(values.size());

            memcpy(data() + m_size, values.data(), sizeof(T) * count);

            m_size += count;

            return { firstIndex, count };
        }

        void clear() noexcept { m_size = 0; }

        auto deviceData() const noexcept -> DeviceAddress { return m_info.gpuAddress; }
        auto getView() const noexcept -> MemoryView { return m_memView; }
    };
};
#endif // LETTUCE_UTILS_GPU_MAPPED_VECTOR