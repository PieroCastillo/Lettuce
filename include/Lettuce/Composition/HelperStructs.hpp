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
        AnimatableProperty objectProperty;
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

    enum class OpCode : uint16_t
    {
        DestroyVisual,
        DestroyMaterial,
        DestroyLight,
        DestroyGeometry,
        DestroyShadow,
        DestroyAnimation,
        DestroyFont,

        AddChild,
        RemoveChild,

        SetVisible,
        SetBlendMode,
        SetRectClip,
        SetRoundedRectClip,
        ClearClip,

        SetMaterial,
        SetMaterialGradientStops,

        UpdateInstancedGeometry,
        UpdatePathGeometry,

        AddLightTarget,
        RemoveLightTarget,
        SetShadowTarget,
        ClearShadowTarget,

        StopAnimations_Visual,
        StopAnimations_Material,
        StopAnimations_Light,
        StopAnimations_Geometry,
        StopAnimations_DropShadow,

        PauseAnimations_Visual,
        PauseAnimations_Material,
        PauseAnimations_Light,
        PauseAnimations_Geometry,
        PauseAnimations_DropShadow,

        ResumeAnimations_Visual,
        ResumeAnimations_Material,
        ResumeAnimations_Light,
        ResumeAnimations_Geometry,
        ResumeAnimations_DropShadow,

        BindImplicitAnimation_Visual,
        BindImplicitAnimation_Material,
        BindImplicitAnimation_Light,
        BindImplicitAnimation_Geometry,
        BindImplicitAnimation_DropShadow,

        SetDebugName,

        SetVisualPosition,
        SetVisualSize,
        SetVisualScale,
        SetVisualRotation,
        SetVisualAnchorPoint,
        SetVisualOpacity,

        SetMaterialColor,
        SetMaterialTintColor,
        SetMaterialBlurRadius,
        SetMaterialNoiseIntensity,
        SetMaterialDistortionStrength,

        SetGeometryCornerRadius,
        SetGeometryStrokeWidth,
        SetGeometryTrimStart,
        SetGeometryTrimEnd,
        SetGeometryTrimOffset,

        SetLightColor,
        SetLightIntensity,
        SetLightPosition,
        SetLightDirection,
        SetLightRadius,

        SetDropShadowColor,
        SetDropShadowBlurRadius,
        SetDropShadowOffset,
        SetDropShadowOpacity,
    };

    using CommandPayload = std::variant<std::monostate, std::array<float, 4>, // float, vec2, vec3, color, offset+count
        AnimatableProperty, std::string,
        bool, BlendMode>;

    enum class CommandPayloadKind : uint16_t
    {
        none,
        boolK,
        float4,
        float5,
        enums,
        stringK,
        arenaRange,
    };

    struct ArenaRange
    {
        uint32_t offset;
        uint32_t byteCount;
    };

    struct CommandPayloadStorage
    {
        std::vector<uint8_t> bools;
        std::vector<std::array<float, 4>> float4;
        std::vector<std::array<float, 5>> float5;
        std::vector<AnimatableProperty>  animProps;
        std::vector<BlendMode> blendModes;
        std::vector<std::string> strings;
        std::vector<uint8_t> arenaStorage;
        std::vector<ArenaRange> arenaRanges;
    };

    struct Command
    {
        OpCode op;
        CommandPayloadKind payloadKind;
        // idx+gen
        std::array<uint32_t, 2> handleDst; // first handle
        std::array<uint32_t, 2> handleSrc; // second handle
        uint32_t payloadIdx;
    };

    class CommandQueue
    {
    public:
        std::vector<Command> appQueue;
        CommandPayloadStorage storage;

        void addCommand(OpCode op, std::array<uint32_t, 2> dst, std::array<uint32_t, 2> src)
        {
            appQueue.emplace_back(Command{ op, CommandPayloadKind::none, dst,  src, 0});
        }

        template<typename PayloadT>
        void addCommand(OpCode op, std::array<uint32_t, 2> dst, std::array<uint32_t, 2> src, const PayloadT& payload)
        {
            auto [kind, idx] = storePayload(payload);
            appQueue.emplace_back(Command{ op, kind, dst, src, idx });
        }
    private:
        auto storePayload(std::monostate) -> std::pair<CommandPayloadKind, uint32_t>
        {
            return { CommandPayloadKind::none, 0 };
        }

        auto storePayload(bool v) -> std::pair<CommandPayloadKind, uint32_t>
        {
            uint32_t idx = storage.bools.size();
            storage.bools.push_back(static_cast<uint8_t>(v));
            return { CommandPayloadKind::boolK, idx };
        }

        auto storePayload(const std::array<float, 4>& v) -> std::pair<CommandPayloadKind, uint32_t>
        {
            uint32_t idx = storage.float4.size();
            storage.float4.push_back(v);
            return { CommandPayloadKind::float4, idx };
        }

        auto storePayload(BlendMode v) -> std::pair<CommandPayloadKind, uint32_t>
        {
            uint32_t idx = storage.blendModes.size();
            storage.blendModes.push_back(v);
            return { CommandPayloadKind::enums, idx };
        }

        auto storePayload(const std::string& v) -> std::pair<CommandPayloadKind, uint32_t>
        {
            uint32_t idx = storage.strings.size();
            storage.strings.push_back(v);
            return { CommandPayloadKind::stringK, idx };
        }

        template<typename T>
        auto storePayload(std::span<const T> span) -> std::pair<CommandPayloadKind, uint32_t>
        {
            uint32_t offset = storage.arenaStorage.size();
            size_t byteCount = span.size_bytes();
            storage.arenaStorage.resize(offset + byteCount);
            memcpy(storage.arenaStorage.data() + offset, span.data(), byteCount);
            uint32_t idx = storage.arenaRanges.size();
            storage.arenaRanges.push_back({ offset,static_cast<uint32_t>(byteCount) });
            return { CommandPayloadKind::arenaRange, idx };
        }
    };
};
#endif // LETTUCE_COMPOSITION_HELPER_STRUCTS_HPP