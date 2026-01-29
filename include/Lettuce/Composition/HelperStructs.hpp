/*
Created by @PieroCastillo on 2026-01-25
*/
#ifndef LETTUCE_COMPOSITION_HELPER_STRUCTS_HPP
#define LETTUCE_COMPOSITION_HELPER_STRUCTS_HPP

// standard headers
#include <vector>

// project headers
#include "api.hpp"

namespace Lettuce::Composition
{
    struct VisualInfo
    {

    };

    struct AnimationTokenInfo
    {

    };

    struct BrushInfo
    {

    };

    struct LightInfo
    {

    };

    struct EffectInfo
    {

    };

    struct VisualGPUData
    {
        Vec2 Position;
        Vec2 Size;
        Vec2 Scale;
        float Rotation;
        Vec2 AnchorPoint;
        float Opacity;
        float CornerRadius;
        float BorderWidth;
    };

    struct AnimationTokenGPUData
    {
        uint32_t durationMs;
        uint32_t delayMs;
        uint32_t iterationCount;
        EasingMode easing;
        bool startFromCurrent;
        float bezierX1;
        float bezierY1;
        float bezierX2;
        float bezierY2;
    };

    struct BrushGPUData
    {        
        BrushType type;
        Color color;
        Color secondaryColor;
        uint32_t textureIdx;
        uint32_t normalMapIdx;
        float metallic;
        float roughness;
        float distortionStrength;
        float noiseScale;
        float noiseIntensity;
    };

    struct LightGPUData
    {        
        LightType type;
        Color color;
        float intensity;
        Vec3 position;
        Vec3 direction;
    };

    struct EffectGPUData
    {        
        EffectType type;
        float intensity;
        Color color;
        Vec2 offset;
        float radius;
        float blurAmount;
        float tintOpacity;
        Color tintColor;
        float noiseIntensity;
        float saturation;
        float brightness;
        float contrast;
        float hueRotation;
    };
};
#endif // LETTUCE_COMPOSITION_HELPER_STRUCTS_HPP