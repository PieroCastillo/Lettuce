/*
Created by @PieroCastillo on 2026-01-25
*/
#ifndef LETTUCE_COMPOSITION_API_HPP
#define LETTUCE_COMPOSITION_API_HPP

#include <chrono>
#include <cstdint>
#include <span>

#include "Core/api.hpp"

using namespace Lettuce::Core;

#define DeclareProperty(HandleType, Name, Type) \
    auto Get##Name(##HandleType handle) -> ##Type; \
    void Set##Name(##HandleType handle, ##Type value); \
    void Set##Name##Animation(##HandleType handle, AnimationToken token); \

namespace Lettuce::Composition
{
    struct AnimationTokenTag {};
    struct VisualTag {};
    struct BrushTag {};
    struct LightTag {};
    struct EffectTag {};

    using AnimationToken = Handle<AnimationTokenTag>;
    using Visual = Handle<VisualTag>;
    using Brush = Handle<BrushTag>;
    using Light = Handle<LightTag>;
    using Effect = Handle<EffectTag>;

    struct Color {
        float r, g, b, a;
    };

    struct Vec2 {
        float x, y;
    };

    struct Vec3 {
        float x, y, z;
    };

    struct Rect {
        float x, y, width, height;
    };

    enum class BrushType : uint8_t {
        SolidColor,
        LinearGradient,
        RadialGradient,
        Texture,
        Acrylic,
        Noise,
    };

    enum class EasingMode : uint8_t {
        Linear,
        EaseIn,
        EaseOut,
        EaseInOut,
        EaseInBack,
        EaseOutBack,
        EaseInOutBack,
        EaseOutBounce,
        EaseOutElastic,
        CubicBezier,
    };

    enum class LightType : uint8_t {
        Ambient,
        Point,
        Spot,
    };

    enum class EffectType : uint8_t {
        Blur,
        DropShadow,
        Glow,
        Saturation,
    };

    enum class BlendMode : uint8_t {
        Normal,
        Multiply,
        Screen,
        Add,
    };

    struct AnimationDesc {
        uint32_t durationMs;
        uint32_t delayMs;
        uint32_t iterationCount;  // 0 = infinite
        EasingMode easing;
        bool startFromCurrent;
        float bezierX1;
        float bezierY1;
        float bezierX2;
        float bezierY2;
    };

    struct NaturalMotionAnimationDesc {
        float stiffness;
        float damping;
        float mass;
    };

    struct BrushDesc {
        BrushType type;
        Color color;
        Color secondaryColor;
        float blurAmount;
        Texture texture;
    };

    struct GradientStop {
        Color color;
        float offset;
    };

    struct LightDesc {
        LightType type;
        Color color;
        float intensity;
        Vec3 position;
        Vec3 direction;
    };

    struct EffectDesc {
        EffectType type;
        float intensity;
        Color color;
        Vec2 offset;
        float radius;
    };

    struct CompositorDesc {
        Device device;
        uint32_t maxVisuals;
        uint32_t maxAnimations;
    };

    struct CompositorImpl;
    class Compositor {
    private:
        CompositorImpl* impl;

    public:
        void Create(const CompositorDesc& desc);
        void Destroy();

        // Visuals
        auto CreateVisual() -> Visual;
        void DestroyVisual(Visual visual);

        void SetVisible(Visual visual, bool visible);
        auto IsVisible(Visual visual) -> bool;
        void SetBlendMode(Visual visual, BlendMode mode);
        void SetRectClip(Visual visual, Rect rect);
        void SetRoundedRectClip(Visual visual, Rect rect, float cornerRadius);
        void ClearClip(Visual visual);

        // Brushes
        auto CreateBrush(const BrushDesc& type) -> Brush;
        void DestroyBrush(Brush brush);

        void SetBrush(Visual visual, Brush brush);
        void SetBorderBrush(Visual visual, Brush brush);

        void SetBrushGradientStops(Brush brush, std::span<const GradientStop> stops);

        // Lights
        auto CreateLight(const LightDesc& desc) -> Light;
        void DestroyLight(Light light);

        void AddLightTarget(Light light, Visual visual);
        void RemoveLightTarget(Light light, Visual visual);

        // Effects
        auto CreateEffect(const EffectDesc& desc) -> Effect;
        void DestroyEffect(Effect effect);

        void AddEffect(Visual visual, Effect effect);
        void RemoveEffect(Visual visual, Effect effect);
        void ClearEffects(Visual visual);

        // Animations
        auto CreateAnimation(const AnimationDesc& desc) -> AnimationToken;
        auto CreateNaturalMotionAnimation(const NaturalMotionAnimationDesc& desc) -> AnimationToken;
        void DestroyAnimation(AnimationToken token);

        void StopAnimations(Visual visual);
        void StopAnimations(Brush brush);
        void StopAnimations(Light light);
        void StopAnimations(Effect effect);
        void StopAnimation(AnimationToken animation);

        void PauseAnimations(Visual visual);
        void PauseAnimations(Brush brush);
        void PauseAnimations(Light light);
        void PauseAnimations(Effect effect);
        void PauseAnimation(AnimationToken animation);
        
        void ResumeAnimations(Visual visual);
        void ResumeAnimations(Brush brush);
        void ResumeAnimations(Light light);
        void ResumeAnimations(Effect effect);
        void ResumeAnimation(AnimationToken animation);

        // Batch Operations
        void BeginBatch();
        void EndBatch();

        void SetOpacityBatch(std::span<const Visual> visuals, float opacity);
        void SetVisibleBatch(std::span<const Visual> visuals, bool visible);

        // Commit
        void Commit();

        // Debug
        void SetDebugName(Visual visual, const char* name);
        auto GetVisualCount() -> uint32_t;
        auto GetAnimationCount() -> uint32_t;

        // Animatable Properties
        DeclareProperty(Visual, Position, Vec2)
        DeclareProperty(Visual, Size, Vec2)
        DeclareProperty(Visual, Scale, Vec2)
        DeclareProperty(Visual, Rotation, float)
        DeclareProperty(Visual, AnchorPoint, Vec2)
        DeclareProperty(Visual, Opacity, float)
        DeclareProperty(Visual, CornerRadius, float)
        DeclareProperty(Visual, BorderWidth, float)

        DeclareProperty(Brush, BrushColor, Color)
        DeclareProperty(Brush, BrushOpacity, float)

        DeclareProperty(Light, LightColor, Color)
        DeclareProperty(Light, LightIntensity, float)
        DeclareProperty(Light, LightPosition, Vec3)
        DeclareProperty(Light, LightDirection, Vec3)

        DeclareProperty(Effect, EffectIntensity, float)
        DeclareProperty(Effect, EffectColor, Color)
        DeclareProperty(Effect, EffectOffset, Vec2)
        DeclareProperty(Effect, EffectRadius, float)
    };
}
#endif // LETTUCE_COMPOSITION_API_HPP