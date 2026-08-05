/*
Created by @PieroCastillo on 2026-08-01
*/
#ifndef LETTUCE_FOUNDATIONS_GPU_UNIQUE_PTR
#define LETTUCE_FOUNDATIONS_GPU_UNIQUE_PTR

// standard headers
#include <utility>

// project headers
#include "../Core/api.hpp"
#include "Common.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Foundations
{
    template<GpuElement T>
    class GpuUniquePtr
    {
    private:
        Device* m_device = nullptr;
        MemoryView m_memView;
        MemoryViewInfo m_info;
    public:
        using value_type = T;

        GpuUniquePtr() noexcept = default;

        /// @brief Constructor
        /// @param device Referred Device
        explicit GpuUniquePtr(Device& device, const T& initialValue = {}) : m_device(&device)
        {
            m_memView = device.CreateMemoryView({ sizeof(T), true });
            m_info = device.GetMemoryViewInfo(m_memView);
            *((T*)m_info.cpuAddress) = initialValue;
        }

        /// @brief Copy Constructor. Deleted.
        GpuUniquePtr(const GpuUniquePtr&) = delete;

        /// @brief Move Constructor.
        GpuUniquePtr(GpuUniquePtr&& other) noexcept :
            m_device(other.m_device),
            m_memView(std::exchange(other.m_memView, {})),
            m_info(std::exchange(other.m_info, {}))
        {
        }

        /// @brief Copy assignment operator. Deleted.
        auto operator=(const GpuUniquePtr&) -> GpuUniquePtr & = delete;

        /// @brief Move assignment operator
        auto operator=(GpuUniquePtr&& other) noexcept -> GpuUniquePtr&
        {
            DebugAssert(m_memView.generation == 0, "GpuUniquePtr cannot be reassigned");

            if (this == &other)
                return *this;

            m_device = std::exchange(other.m_device, nullptr);
            m_memView = std::exchange(other.m_memView, {});
            m_info = std::exchange(other.m_info, {});

            return *this;
        }

        /// @brief Destructor.
        ~GpuUniquePtr()
        {
            if (m_memView.generation == 0)
                return;

            m_device->Destroy(m_memView);
            m_memView = {};
            m_info = {};
        }

        auto byteSize() const noexcept -> uint32_t { return sizeof(T); }
        auto size() const noexcept -> uint32_t { return 1; }

        auto data() noexcept -> T* { return (T*)m_info.cpuAddress; }
        auto data() const noexcept -> const T* { return (T*)m_info.cpuAddress; }

        auto deviceData() const noexcept -> DeviceAddress { return m_info.gpuAddress; }
        auto getView() const noexcept -> MemoryView { return m_memView; }

        void reset() noexcept
        {
            if (m_memView.generation == 0)
                return;

            m_device->Destroy(m_memView);

            m_device = nullptr;
            m_memView = {};
            m_info = {};
        }

        T& operator*() noexcept
        {
            DebugAssert(*this, "Dereferencing null GpuUniquePtr");
            return *data();
        }
        const T& operator*() const noexcept
        {
            DebugAssert(*this, "Dereferencing null GpuUniquePtr");
            return *data();
        }

        T* operator->() noexcept
        {
            DebugAssert(*this, "Dereferencing null GpuUniquePtr");
            return data();
        }
        const T* operator->() const noexcept
        {
            DebugAssert(*this, "Dereferencing null GpuUniquePtr");
            return data();
        }

        explicit operator bool() const noexcept { return m_memView.generation != 0; }
    };
}
#endif // LETTUCE_FOUNDATIONS_GPU_UNIQUE_PTR