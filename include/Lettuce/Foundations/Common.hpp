#ifndef LETTUCE_FOUNDATIONS_COMMON_HPP
#define LETTUCE_FOUNDATIONS_COMMON_HPP

// standard headers
#include <concepts>
#include <cstdint>
#include <type_traits>

// project headers
#include "../Core/api.hpp"

namespace Lettuce::Foundations
{
    template<class T>
    concept GpuElement = std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>;

    template<class V, class T>
    concept GpuSpanCompatible = GpuElement<typename V::value_type> && std::same_as<typename V::value_type, T> &&
    requires(const V& v)
    {
        { v.getView() } -> std::same_as<Lettuce::Core::MemoryView>;
        { v.size() } -> std::convertible_to<uint32_t>;
    };
}
#endif // LETTUCE_FOUNDATIONS_COMMON_HPP