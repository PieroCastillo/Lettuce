/*
Created by @PieroCastillo on 2026-06-01
*/
#ifndef LETTUCE_UTILS_GPU_UPLOAD_VECTOR
#define LETTUCE_UTILS_GPU_UPLOAD_VECTOR

// standard headers
#include <atomic>

// project headers
#include "../Core/api.hpp"
#include "../helper.hpp"
#include "Common.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Utils
{
    enum class UploadState : uint8_t
    {
        Writable,
        Submitted,
        Ready,
    };

    template<GpuElement T>
    class GpuUploadVector
    {
    private:
        Device* m_device = {};
        MemoryView m_memView = {};
        MemoryViewInfo m_info = {};
        MemoryView m_tempView = {};
        MemoryViewInfo m_tempInfo = {};
        uint32_t m_size = {};
        uint32_t m_capacity = {};
        std::atomic<UploadState> m_state = { UploadState::Writable };
        void assertWritable() const noexcept
        {
            DebugAssert(m_state.load(std::memory_order_acquire) == UploadState::Writable, "GpuUploadVector: temporary CPU memory is no longer writable/readable after Upload().");
        }

        void assertReady() const noexcept
        {
            DebugAssert(m_state.load(std::memory_order_acquire) == UploadState::Ready, "GpuUploadVector: deviceData() is only valid after upload finished.");
        }
    public:
        using iterator = T*;
        using const_iterator = const T*;

        GpuUploadVector() noexcept = default;

        explicit GpuUploadVector(Device& device, uint32_t capacity)
            : m_device(&device), m_capacity(capacity), m_size(0)
        {
            DebugAssert(capacity > 0, "Capacity MUST be greater than 0");

            m_memView = device.CreateMemoryView({ sizeof(T) * capacity, false });
            m_info = device.GetMemoryViewInfo(m_memView);

            m_tempView = device.CreateMemoryView({ sizeof(T) * capacity, true });
            m_tempInfo = device.GetMemoryViewInfo(m_tempView);
        }

        GpuUploadVector(const GpuUploadVector&) = delete;

        GpuUploadVector(GpuUploadVector&& other) noexcept :
            m_device(std::exchange(other.m_device, nullptr)),
            m_memView(std::exchange(other.m_memView, {})),
            m_info(std::exchange(other.m_info, {})),
            m_tempView(std::exchange(other.m_tempView, {})),
            m_tempInfo(std::exchange(other.m_tempInfo, {})),
            m_size(std::exchange(other.m_size, 0)),
            m_capacity(std::exchange(other.m_capacity, 0)),
            m_state(other.m_state.load(std::memory_order_acquire))
        {
            DebugAssert(other.m_state.load(std::memory_order_acquire) != UploadState::Submitted, "Cannot move GpuUploadVector while upload is submitted.");
            other.m_state.store(UploadState::Writable, std::memory_order_release);
        }

        auto operator=(const GpuUploadVector&) -> GpuUploadVector & = delete;
        auto operator=(GpuUploadVector&&) -> GpuUploadVector & = delete;

        ~GpuUploadVector()
        {
            if (m_device && m_memView.generation > 0)
                m_device->Destroy(m_memView);

            if (m_device && m_tempView.generation > 0)
                m_device->Destroy(m_tempView);

            m_info = {};
            m_tempInfo = {};
            m_size = 0;
            m_capacity = 0;
        }

        auto data() noexcept -> T*
        {
            assertWritable();
            return reinterpret_cast<T*>(m_tempInfo.cpuAddress);
        }
        auto data() const noexcept -> const T*
        {
            assertWritable();
            return reinterpret_cast<const T*>(m_tempInfo.cpuAddress);
        }

        auto begin() noexcept -> iterator {
            assertWritable();
            return data();
        }
        auto end() noexcept -> iterator {
            assertWritable();
            return data() + m_size;
        }
        auto begin() const noexcept -> const_iterator {
            assertWritable();
            return data();
        }
        auto end() const noexcept -> const_iterator {
            assertWritable();
            return data() + m_size;
        }

        auto operator[](uint32_t index) noexcept -> T& {
            assertWritable();
            DebugAssert(index < m_size, "Index out of bounds.");
            return ((T*)m_tempInfo.cpuAddress)[index];
        }
        auto operator[](uint32_t index) const noexcept -> const T& {
            assertWritable();
            DebugAssert(index < m_size, "Index out of bounds.");
            return ((T*)m_tempInfo.cpuAddress)[index];
        }

        auto first() noexcept -> T& {
            assertWritable();
            DebugAssert(m_size > 0, "GpuUploadVector is empty.");
            return ((T*)m_tempInfo.cpuAddress)[0];
        }
        auto last() noexcept -> T& {
            assertWritable();
            DebugAssert(m_size > 0, "GpuUploadVector is empty.");
            return ((T*)m_tempInfo.cpuAddress)[m_size - 1];
        }
        auto first() const noexcept -> const T& {
            assertWritable();
            DebugAssert(m_size > 0, "GpuUploadVector is empty.");
            return ((T*)m_tempInfo.cpuAddress)[0];
        }
        auto last() const noexcept -> const T& {
            assertWritable();
            DebugAssert(m_size > 0, "GpuUploadVector is empty.");
            return ((T*)m_tempInfo.cpuAddress)[m_size - 1];
        }

        auto push_back(const T& value) -> uint32_t
        {
            DebugAssert(m_size < m_capacity, "size MUST be less than capacity to push a value.");
            assertWritable();

            const uint32_t index = m_size;
            data()[index] = value;
            ++m_size;

            return index;
        }

        auto append(std::span<const T> values) -> std::pair<uint32_t, uint32_t>
        {
            DebugAssert(m_size + values.size() <= m_capacity, "total size MUST be less or equal than capacity");
            assertWritable();

            const uint32_t firstIndex = m_size;
            const uint32_t count = static_cast<uint32_t>(values.size());

            memcpy(data() + m_size, values.data(), sizeof(T) * count);

            m_size += count;

            return { firstIndex, count };
        }

        void clear() noexcept
        {
            assertWritable();
            m_size = 0;
        }

        /// @brief Upload Data to device memory. It's a blocking operation.
        void Upload()
        {
            DebugAssert(m_device != nullptr, "GpuUploadVector is not initialized.");

            assertWritable();
            m_state.store(UploadState::Submitted, std::memory_order_release);

            MemoryToMemoryCopy copy = {
                .srcMemory = m_tempView,
                .dstMemory = m_memView,
                .size = sizeof(T) * m_size,
                .srcOffset = 0,
                .dstOffset = 0,
            };

            m_device->MemoryCopy(copy);

            m_device->Destroy(m_tempView);
            m_tempView = {};
            m_tempInfo = {};

            m_state.store(UploadState::Ready, std::memory_order_release);
        }

        auto deviceData() const noexcept -> DeviceAddress
        {
            assertReady();
            return m_info.gpuAddress;
        }
        auto getView() const noexcept -> MemoryView
        {
            assertReady(); 
            return m_memView;
        }
    };
};
#endif // LETTUCE_UTILS_GPU_UPLOAD_VECTOR