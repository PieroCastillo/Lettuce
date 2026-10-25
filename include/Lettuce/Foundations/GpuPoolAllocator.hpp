/*
Created by @PieroCastillo on 2026-25-10
*/
#ifndef LETTUCE_FOUNDATIONS_GPU_POOL_ALLOCATOR_HPP
#define LETTUCE_FOUNDATIONS_GPU_POOL_ALLOCATOR_HPP

// standard headers
#include <vector>

// project headers
#include "../Core/api.hpp"
#include "../helper.hpp"
#include "Common.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Foundations
{
    template<GpuElement T>
    struct GpuPoolAllocator
    {
    private:
        Device* m_device = nullptr;
        MemoryView m_memory;
        HostAddress m_addr;
        uint32_t m_offset;
        uint32_t m_capacity;
        std::vector<uint32_t> m_freeIndices;
        static constexpr uint32_t elementSize = sizeof(T);
    public:
        GpuPoolAllocator() = default;

        /// @brief Destructor.
        ~GpuPoolAllocator();

        explicit GpuPoolAllocator(Device& pDevice, uint32_t capacity) : m_offset(0), m_capacity(capacity), m_device(&pDevice)
        {
            m_memory = m_device->CreateMemoryView({ capacity * sizeof(T), true });
            m_addr = m_device->GetMemoryViewInfo(m_memory).cpuAddress;
        }

        /// @brief Copy Constructor. Deleted.
        GpuPoolAllocator(const GpuPoolAllocator&) = delete;

        /// @brief Copy assignment operator. Deleted.
        auto operator=(const GpuPoolAllocator&) -> GpuPoolAllocator & = delete;

        /// @brief Move Constructor.
        GpuPoolAllocator(GpuPoolAllocator&& other) noexcept :
            m_device(std::exchange(other.m_device, nullptr)),
            m_memory(std::exchange(other.m_memory, {})),
            m_addr(std::exchange(other.m_addr, nullptr)),
            m_offset(std::exchange(other.m_offset, 0)),
            m_capacity(std::exchange(other.m_capacity, 0)),
            m_freeIndices(std::exchange(other.m_freeIndices, {}))
        {
        }

        /// @brief Move assignment operator
        auto operator=(GpuPoolAllocator&& other) noexcept -> GpuPoolAllocator&
        {
            DebugAssert(m_memory.generation == 0, "GpuPoolAllocator cannot be reassigned");

            if (this == &other)
                return *this;

            m_device = std::exchange(other.m_device, nullptr);
            m_memory = std::exchange(other.m_memory, {});
            m_addr = std::exchange(other.m_addr, nullptr);
            m_offset = std::exchange(other.m_offset, 0);
            m_capacity = std::exchange(other.m_capacity, 0);
            m_freeIndices = std::exchange(other.m_freeIndices, {});

            return *this;
        }

        auto size() const noexcept -> uint32_t { return m_capacity * elementSize; }
        auto getView() const noexcept -> MemoryView { return m_memory; }
        uint32_t Push(const T& value);
        void Free(uint32_t index);
        void reset();
    };

    template<GpuElement T>
    GpuPoolAllocator<T>::~GpuPoolAllocator()
    {
        reset();
    }

    template<GpuElement T>
    uint32_t GpuPoolAllocator<T>::Push(const T& value)
    {
        uint32_t index;

        if (!m_freeIndices.empty())
        {
            index = m_freeIndices.back();
            m_freeIndices.pop_back();
        }
        else
        {
            index = m_offset++;
        }

        reinterpret_cast<T*>(m_addr)[index] = value;
        return index;
    }

    template<GpuElement T>
    void GpuPoolAllocator<T>::Free(uint32_t index)
    {
        reinterpret_cast<T*>(m_addr)[index] = {};
        m_freeIndices.push_back(index);
    }

    template<GpuElement T>
    void GpuPoolAllocator<T>::reset()
    {
        if (!m_device || m_memory.generation == 0)
            return;

        m_device->Destroy(m_memory);
        m_memory = {};
        m_addr = 0;
        m_offset = 0;
        m_capacity = 0;
        m_freeIndices = {};
        m_device = nullptr;
    }
};
#endif // LETTUCE_FOUNDATIONS_GPU_POOL_ALLOCATOR_HPP