/*
Created by @PieroCastillo on 2026-11-14
*/
#ifndef LETTUCE_RENDERING_LIGHTING_TYPES_HPP
#define LETTUCE_RENDERING_LIGHTING_TYPES_HPP

// standard headers
#include <cstdint>
#include <type_traits>

// project headers
#include "../Core/basicTypes.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Rendering
{
    constexpr uint32_t MaxSpotLights = 64;
    constexpr uint32_t MaxCascades = 4;

    struct SunLightData
    {
        float3 direction; // surface to sun direction
        float intensity;
        float3 color;
        float _padding;
    };

    struct SpotLightStorage
    {
        float3 position;
        float range;
        float3 direction; // emission direction
        float intensity;
        float3 color;
        float innerCos;
        float outerCos;

        float3 padding;
    };

    struct SpotLightData
    {
        uint32_t count;
        float4 padding;
        SpotLightStorage lights[MaxSpotLights]; // max count: MaxSpotLights
    };

    struct CascadeShadowMapData
    {
        float4 splitDepths; // cascade view-space depth
        float shadowBias;
        float normalBias;
        float texelSize;
        uint32_t cascadeCount;
        uint32_t shadowTextureID;
        float4x4 worldToShadow[MaxCascades]; // max count: MaxCascades
    };

    // checks
    static_assert(std::is_trivially_copyable_v<SpotLightData>, "SpotLightData MUST be trivially copyable");
    static_assert(std::is_trivially_copyable_v<CascadeShadowMapData>, "CascadeShadowMapData MUST be trivially copyable");
};
#endif // LETTUCE_RENDERING_LIGHTING_TYPES_HPP