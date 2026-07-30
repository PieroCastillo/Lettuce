#ifndef LETTUCE_FOUNDATIONS_COMMON_HPP
#define LETTUCE_FOUNDATIONS_COMMON_HPP

#include <concepts>
#include <type_traits>

namespace Lettuce::Foundations
{
    template<class T>
    concept GpuElement = std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>;
}
#endif // LETTUCE_FOUNDATIONS_COMMON_HPP