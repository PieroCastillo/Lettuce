/*
Created by @PieroCastillo on 2026-01-25
*/
#ifndef LETTUCE_COMPOSITION_API_HPP
#define LETTUCE_COMPOSITION_API_HPP

#include <chrono>
#include <cstdint>
#include <span>
#include <string>

#include "../Core/api.hpp"

using namespace Lettuce::Core;

#define DeclareAnimatableProperty(HandleType, Name, Type) \
    auto Get##Name(##HandleType handle) -> ##Type; \
    void Set##Name(##HandleType handle, ##Type value); \

namespace Lettuce::Composition
{
    struct AnimationTokenTag {};
    struct VisualTag {};
    struct MaterialTag {};
    struct LightTag {};
    struct DropShadowTag {};
    struct GeometryTag {};
    struct FontTag {};

    using AnimationToken = Handle<AnimationTokenTag>;
    using Visual = Handle<VisualTag>;
    using Material = Handle<MaterialTag>;
    using Light = Handle<LightTag>;
    using DropShadow = Handle<DropShadowTag>;
    using Geometry = Handle<GeometryTag>;
    using Font = Handle<FontTag>;

    enum class LightType : uint8_t {
        Ambient,
        Point,
        Spot,
    };

    enum class BlendMode : uint8_t {
        Normal,
        Multiply,
        Screen,
        Add,
    };

    enum class GeometryFillRule : uint8_t {
        EvenOdd,
        NonZero
    };

    enum class ImplicitShape : uint8_t {
        Rectangle,
        RoundedRectangle,
        Ellipse,
    };

    enum class EasingMode : uint8_t {
        Linear,
        EaseIn,
        EaseOut,
        EaseInOut,
        EaseInBack,
        EaseOutBack,
        EaseInOutBack,
        Bounce,
        Elastic,
        CustomBezier,
    };

    enum class FontWeight : uint16_t {
        Thin = 100,
        Light = 300,
        Regular = 400,
        Medium = 500,
        SemiBold = 600,
        Bold = 700,
        Black = 900,
    };

    enum class AtlasInstanceFlags : uint8_t {
        None = 0,
        IsColorBitmap = 1 << 0,
        IsSubpixelAA = 1 << 1,
    };

    enum class AnimatableProperty : uint16_t {
        Position = 0,
        Size,
        Scale,
        Rotation,
        AnchorPoint,
        Opacity,

        CornerRadius = 100, // Parametric Geometry
        StrokeWidth,        // Parametric / Vector Geometry
        TrimStart,          // Vector Geometry
        TrimEnd,            // Vector Geometry
        TrimOffset,         // Vector Geometry
        MorphProgress,      // Morph Geometry

        Color = 200,        // SolidColor
        SecondaryColor,     // Gradients
        TintColor,          // Glass / Acrylic
        BlurRadius,         // Glass / Acrylic
        NoiseIntensity,     // Noise / Acrylic
        DistortionStrength, // Distortion Map

        LightIntensity = 300,
        LightColor,
        LightPosition,      // Point / Spot / Reveal
        LightDirection,     // Spot / Directional
        LightRadius,        // Reveal Light

        ShadowColor = 400,
        ShadowBlurRadius,
        ShadowOffset,
        ShadowOpacity
    };

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

    struct GradientStop {
        Color color;
        float offset;
    };

    struct AtlasInstance {
        Rect uvCrop;
        Rect localRect;
        Color tint;
        AtlasInstanceFlags flags;
    };

    struct GlyphUpload {
        Rect destinationRect;
        const byte* pixelData;
        size_t dataSize;
    };

    struct PathCommand {
        uint8_t type;
        Vec2 points[3];
    };

    // Visual Descriptions
    struct ContainerVisualDesc {
        Vec2 size = { 0, 0 };
        bool clipsToBounds = false;
    };

    struct SpriteVisualDesc {
        Vec2 size = { 0, 0 };
        Geometry geometry;
        Material material;
    };

    // Material Descriptions
    struct SolidColorMaterialDesc {
        Color color;
    };

    struct LinearGradientMaterialDesc {
        Vec2 startPoint;
        Vec2 endPoint;
        std::span<const GradientStop> stops;
    };

    struct RadialGradientMaterialDesc {
        Vec2 center;
        Vec2 gradientOriginOffset;
        float radiusX, radiusY;
        std::span<const GradientStop> stops;
    };

    struct MeshGradientMaterialDesc {
        std::span<const Color> colors;
        std::span<const Vec2> positions;
    };

    struct TextureMaterialDesc {
        Texture texture;
        BlendMode blendMode = BlendMode::Normal;
        float opacity = 1.0f;
    };

    // Backdrop Material Descriptions
    struct GlassMaterialDesc {
        float blurRadius = 15.0f;
        Color tintColor = { 1, 1, 1, 0.2f };
        float specularIntensity = 0.5f;
    };

    struct AcrylicMaterialDesc {
        float blurRadius = 30.0f;
        Color tintColor = { 0, 0, 0, 0.6f };
        float noiseIntensity = 0.02f;
        float luminosityOpacity = 0.8f;
    };

    struct NoiseMaterialDesc {
        float scale;
        float intensity;
    };

    struct DistortionMaterialDesc {
        Texture displacementMap;
        float strength;
    };

    // Geometry Descriptions
    struct ImplicitGeometryDesc {
        ImplicitShape shape = ImplicitShape::Rectangle;
        float cornerRadius = 0.0f;
        float strokeWidth = 0.0f;
    };

    struct InstancedGeometryDesc {
        Texture atlasTexture;
        std::span<const AtlasInstance> instances;
    };

    struct PathGeometryDesc {
        std::span<const PathCommand> commands;
        GeometryFillRule fillRule = GeometryFillRule::EvenOdd;
    };

    // Light & Shadow Descriptions
    struct AmbientLightDesc {
        Color color;
        float intensity;
    };

    struct DirectionalLightDesc {
        Color color;
        float intensity;
        Vec2 direction;
        float elevation;
    };

    struct SpotLightDesc {
        Color color;
        float intensity;
        Vec3 position;
        Vec3 direction;
        float innerConeAngle;
        float outerConeAngle;
    };

    struct RevealLightDesc {
        Color color;
        float intensity;
        Vec2 position;
        float radius;
        float falloff;
    };

    struct DropShadowDesc {
        Color color = { 0, 0, 0, 0.5f };
        float blurRadius = 10.0f;
        Vec2 offset = { 0.0f, 4.0f };
    };

    struct AnimationDesc {
        uint32_t durationMs = 300;
        uint32_t delayMs = 0;
        uint32_t iterationCount = 1; // 0 = loop
        EasingMode easing = EasingMode::EaseOut;

        // if easing == CustomBezier
        float bezierX1 = 0.25f;
        float bezierY1 = 0.1f;
        float bezierX2 = 0.25f;
        float bezierY2 = 1.0f;
    };

    struct NaturalMotionAnimationDesc {
        float stiffness = 200.0f;
        float damping = 20.0f;
        float mass = 1.0f;
        float initialVelocity = 0.0f;
    };

    struct FontDesc {
        std::string_view familyName;
        float baseSize;
        FontWeight weight = FontWeight::Regular;
        bool italic = false;

        const byte* memoryData = nullptr;
        size_t memorySize = 0;
    };

    struct CompositorDesc {
        Device* device;
        Swapchain swapchain;
        uint32_t maxAnimations;
        uint32_t maxLights;
        uint32_t maxLinkedTextures;
        uint32_t maxMaterials;
        uint32_t maxVisuals;
    };

    struct CompositorImpl;
    class Compositor {
    private:
        CompositorImpl* impl;

    public:
        void Create(const CompositorDesc& desc);
        void Destroy();

        // Visuals
        auto CreateVisual(const ContainerVisualDesc&) -> Visual;
        auto CreateVisual(const SpriteVisualDesc&) -> Visual;
        void DestroyVisual(Visual visual);

        void AddChild(Visual parent, Visual child);
        void RemoveChild(Visual parent, Visual child);

        void SetVisible(Visual visual, bool visible);
        auto IsVisible(Visual visual) -> bool;
        void SetBlendMode(Visual visual, BlendMode mode);
        void SetRectClip(Visual visual, Rect rect);
        void SetRoundedRectClip(Visual visual, Rect rect, float cornerRadius);
        void ClearClip(Visual visual);

        auto HitTest(Vec2 screenPoint) -> Visual;

        // Fonts
        auto CreateFont(const FontDesc& desc) -> Font;
        void DestroyFont(Font font);

        void UploadToAtlasTexture(Texture atlas, std::span<const GlyphUpload> uploads);

        // Materials
        auto CreateMaterial(const SolidColorMaterialDesc&) -> Material;
        auto CreateMaterial(const LinearGradientMaterialDesc&) -> Material;
        auto CreateMaterial(const RadialGradientMaterialDesc&) -> Material;
        auto CreateMaterial(const MeshGradientMaterialDesc&) -> Material;
        auto CreateMaterial(const TextureMaterialDesc&) -> Material;
        auto CreateMaterial(const GlassMaterialDesc&) -> Material;
        auto CreateMaterial(const AcrylicMaterialDesc&) -> Material;
        auto CreateMaterial(const NoiseMaterialDesc&) -> Material;
        auto CreateMaterial(const DistortionMaterialDesc&) -> Material;
        void DestroyMaterial(Material Material);

        void SetMaterial(Visual visual, Material Material);
        void SetMaterialGradientStops(Material Material, std::span<const GradientStop> stops);

        // Geometries
        auto CreateGeometry(const ImplicitGeometryDesc&) -> Geometry;
        auto CreateGeometry(const InstancedGeometryDesc&) -> Geometry;
        auto CreateGeometry(const PathGeometryDesc&) -> Geometry;
        void DestroyGeometry(Geometry);

        void UpdateInstancedGeometry(Geometry geom, std::span<const AtlasInstance> instances);
        void UpdatePathGeometry(Geometry geom, std::span<const PathCommand> commands);

        // Lights
        auto CreateLight(const AmbientLightDesc&) -> Light;
        auto CreateLight(const DirectionalLightDesc&) -> Light;
        auto CreateLight(const SpotLightDesc&) -> Light;
        auto CreateLight(const RevealLightDesc&) -> Light;
        void DestroyLight(Light light);

        void AddLightTarget(Light light, Visual visual);
        void RemoveLightTarget(Light light, Visual visual);

        // Drop Shadows
        auto CreateDropShadow(const DropShadowDesc&) -> DropShadow;
        void DestroyDropShadow(DropShadow);

        void SetShadowTarget(Visual visual, DropShadow shadow);
        void ClearShadowTarget(Visual visual);

        // Animations
        auto CreateAnimation(const AnimationDesc& desc) -> AnimationToken;
        auto CreateAnimation(const NaturalMotionAnimationDesc& desc) -> AnimationToken;
        void DestroyAnimation(AnimationToken token);

        void StopAnimations(Visual visual);
        void StopAnimations(Material Material);
        void StopAnimations(Light light);

        void PauseAnimations(Visual visual);
        void PauseAnimations(Material Material);
        void PauseAnimations(Light light);

        void ResumeAnimations(Visual visual);
        void ResumeAnimations(Material Material);
        void ResumeAnimations(Light light);

        void BindImplicitAnimation(Visual visual, AnimatableProperty prop, AnimationToken token);
        void BindImplicitAnimation(Material material, AnimatableProperty prop, AnimationToken token);
        void BindImplicitAnimation(Geometry geometry, AnimatableProperty prop, AnimationToken token);
        void BindImplicitAnimation(Light light, AnimatableProperty prop, AnimationToken token);
        void BindImplicitAnimation(DropShadow shadow, AnimatableProperty prop, AnimationToken token);

        // Commit
        void Commit();

        // Debug
        void SetDebugName(Visual visual, std::string name);
        auto GetVisualCount() -> uint32_t;
        auto GetAnimationCount() -> uint32_t;

        // Visual Animatable Properties
        DeclareAnimatableProperty(Visual, Position, Vec2);
        DeclareAnimatableProperty(Visual, Size, Vec2);
        DeclareAnimatableProperty(Visual, Scale, Vec2);
        DeclareAnimatableProperty(Visual, Rotation, float); // Rads
        DeclareAnimatableProperty(Visual, AnchorPoint, Vec2); // Normalized 
        DeclareAnimatableProperty(Visual, Opacity, float);

        // Material Animatable Properties
        DeclareAnimatableProperty(Material, Color, Color); // For SolidColor only
        DeclareAnimatableProperty(Material, TintColor, Color); // For Acrylic/Glass only
        DeclareAnimatableProperty(Material, BlurRadius, float); // For Acrylic/Glass only
        DeclareAnimatableProperty(Material, NoiseIntensity, float);
        DeclareAnimatableProperty(Material, DistortionStrength, float);

        // Geometry Animatable Properties
        DeclareAnimatableProperty(Geometry, CornerRadius, float); // ImplicitGeometry only
        DeclareAnimatableProperty(Geometry, StrokeWidth, float);

        // For Trim Paths
        DeclareAnimatableProperty(Geometry, TrimStart, float);  // Normalized
        DeclareAnimatableProperty(Geometry, TrimEnd, float);    // Normalized
        DeclareAnimatableProperty(Geometry, TrimOffset, float); // Normalized

        // Light Animatable Properties
        DeclareAnimatableProperty(Light, Color, Color);
        DeclareAnimatableProperty(Light, Intensity, float);
        DeclareAnimatableProperty(Light, Position, Vec3);  // Point/Spot/Reveal
        DeclareAnimatableProperty(Light, Direction, Vec3); // Directional/Spot
        DeclareAnimatableProperty(Light, Radius, float);   // RevealLight only

        // DropShadow Animatable Properties
        DeclareAnimatableProperty(DropShadow, Color, Color);
        DeclareAnimatableProperty(DropShadow, BlurRadius, float);
        DeclareAnimatableProperty(DropShadow, Offset, Vec2);
        DeclareAnimatableProperty(DropShadow, Opacity, float);
    };
};
#endif // LETTUCE_COMPOSITION_API_HPP