/*
Created by @PieroCastillo on 2026-07-23
*/
#ifndef LETTUCE_CORE_BASIC_TYPES_HPP
#define LETTUCE_CORE_BASIC_TYPES_HPP

#include <array>
#include <cstdint>

// external libs
#include <glm/glm.hpp>

namespace Lettuce::Core
{
    using float2 = glm::vec2;
    using float3 = glm::vec3;
    using float4 = glm::vec4;
    using float3x3 = glm::mat3;
    using float4x4 = glm::mat4;
    using DeviceAddress = uint64_t;
    using HostAddress = uint8_t*;

    template<typename Tag>
    struct Handle {
        uint32_t index = 0;
        uint32_t generation = 0;

        static constexpr Handle<Tag> Null() noexcept {
            return {};
        }

        constexpr bool valid() const noexcept {
            return generation != 0;
        }

        auto operator<=>(const Handle&) const = default;

        constexpr std::array<uint32_t, 2> get() const noexcept {
            return { index, generation };
        }
    };
};
#endif // LETTUCE_CORE_BASIC_TYPES_HPP