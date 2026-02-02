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
    enum class AnimatableProperties : uint8_t
    {
        // Visual Props
        Position,
        Size,
        Scale,
        Rotation,
        AnchorPoint,
        Opacity,
        CornerRadius,
        BorderWidth,
        // Light Props
        LightColor,
        LightIntensity,
        LightPosition,
        LightDirection,
        // Brush Props
        BrushColor,
        BrushSecondaryColor,
        BrushRoughness,
        BrushDistortionStrength,
        BrushNoiseScale,
        BrushNoiseIntensity,
        // Effects Props
        EffectIntensity,
        EffectColor,
        EffectOffset,
        EffectRadius,
        EffectBlurAmount,
        EffectTintOpacity,
        EffectTintColor,
    };

    struct VisualInfo
    {
        uint32_t gpuIdx;
    };

    struct AnimationTokenInfo
    {
        uint32_t gpuIdx;
        uint32_t durationMs;
        uint32_t delayMs;
        uint32_t iterationCount;
        bool startFromCurrent;
    };

    struct BrushInfo
    {
        uint32_t gpuIdx;
    };

    struct LightInfo
    {
        uint32_t gpuIdx;
    };

    struct EffectInfo
    {
        uint32_t gpuIdx;
    };

    struct DataBufferGPULayout
    {
        uint64_t animationInstanceCount;
        uint64_t animationTokenCount;
        uint64_t visualCount;
        uint64_t brusheCount;
        uint64_t lightCount;
        uint64_t effectCount;
    };

    struct AnimationInstanceGPUData
    {
        AnimatableProperties objectProperty;
        uint32_t objectIdx;
        uint32_t animationTokenIdx;
        float startValue[4];
        float targetValue[4];
        bool naturalMotion;
        double startTime;
        double duration;
    };

    struct AnimationTokenGPUData
    {
        EasingMode easing;
        float bezierX1;
        float bezierY1;
        float bezierX2;
        float bezierY2;

        float stiffness;
        float damping;
        float mass;
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

    struct BrushGPUData
    {
        BrushType type;
        Color color;
        Color secondaryColor;
        uint32_t textureIdx;
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
    };
};
#endif // LETTUCE_COMPOSITION_HELPER_STRUCTS_HPP