/*
Created by @PieroCastillo on 2026-08-01
*/
#ifndef LETTUCE_FOUNDATIONS_GPU_SPAN
#define LETTUCE_FOUNDATIONS_GPU_SPAN

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
    class GpuSpan
    {
    private:
        MemoryView m_view{};
        uint32_t m_offset{};
        uint32_t m_size{};
    public:
        GpuSpan() noexcept = default;

        GpuSpan(MemoryView view, uint32_t size, uint32_t offset = 0) noexcept : m_view(view), m_size(size), m_offset(offset)
        {
        }

        template<GpuSpanCompatible<T> V>
        GpuSpan(const V& v) noexcept : m_view(v.getView()), m_offset(0), m_size(v.size())
        {
        }

        auto size() const noexcept -> uint32_t { return m_size; }
        auto empty() const noexcept -> bool { return m_size == 0; }
        auto offset() const noexcept -> uint32_t { return m_offset; }

        auto byteOffset() const noexcept -> uint32_t { return sizeof(T) * m_offset; }
        auto byteSize() const noexcept -> uint32_t { return sizeof(T) * m_size; }
        auto getView() const noexcept -> MemoryView { return m_view; }
    };

    template<class V>
    requires GpuSpanCompatible<V, typename V::value_type>
    GpuSpan(const V&) -> GpuSpan<typename V::value_type>;
};
#endif // LETTUCE_FOUNDATIONS_GPU_SPAN