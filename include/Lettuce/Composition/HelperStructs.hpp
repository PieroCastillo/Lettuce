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
        // Material Props
        MaterialColor,
        MaterialSecondaryColor,
        MaterialRoughness,
        MaterialDistortionStrength,
        MaterialNoiseScale,
        MaterialNoiseIntensity,
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

    struct MaterialInfo
    {
        uint32_t gpuIdx;
    };

    struct LightInfo
    {
        uint32_t gpuIdx;
    };

    struct DataBufferGPULayout
    {
        uint64_t animationInstanceCount;
        uint64_t animationTokenCount;
        uint64_t visualCount;
        uint64_t MaterialeCount;
        uint64_t lightCount;
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

    struct MaterialGPUData
    {
        Material type;
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
};
#endif // LETTUCE_COMPOSITION_HELPER_STRUCTS_HPP