/*
Created by @PieroCastillo on 2026-06-29
*/
#ifndef LETTUCE_QUIMERA_API_HPP
#define LETTUCE_QUIMERA_API_HPP

#include <chrono>
#include <cstdint>
#include <span>
#include <string>

#include "../Core/api.hpp"
#include "colors.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Quimera
{
    struct GeometryTag {};
    struct BrushTag {};
    struct LightTag {};
    struct LayoutTag {};
    struct AnimationTag {};

    using Geometry = Handle<GeometryTag>;
    using Brush = Handle<BrushTag>;
    using Light = Handle<LightTag>;
    using Layout = Handle<LayoutTag>;
    using Animation = Handle<AnimationTag>;

    struct Size { float w, h; };
    struct Rect { float x, y, w, h; };
    struct Extend { uint32_t x, y, w, h; };

    enum class SolidColorBrushProperties
    {
        Color,
    };

    enum class ImplicitGeometryProperties
    {
        Width, Height,
        LeftTop, LeftBottom,
        RightTop, RightBottom,
    };

    enum class LayoutProperties
    {
        Position,
        Scale,
        Skew,
        AnchorPoint,
        Rotation,
    };

    struct SolidColorBrushDesc
    {
        float4 color;
    };

    struct ImplicitGeometryDesc
    {
        Size size;
        float leftTopCornerRadious;
        float leftBottomCornerRadious;
        float rightTopCornerRadious;
        float rightBottomCornerRadious;
    };

    struct LayoutDesc
    {
        float2 position;
        float2 scale;
        float2 skew;
        float2 anchorPoint;
        float rotation;
    };

    struct NaturalMotionAnimationDesc
    {
        float mass;
        float stiffness;
        float dumping;
    };

    struct DrawSurfaceDesc
    {
        TextureView dstTexture;
        TextureView dstDepthTexture;
        Extend renderArea;
    };

    struct SurfaceDesc
    {
        Device& device;

        uint32_t maxAnimations;
        uint32_t maxImplicitGeometries;
        uint32_t maxBrushes;
        uint32_t maxDrawCommands;
        Format colorOutputFormat;
    };

    struct SurfaceImpl;
    struct SurfaceCommandBuffer;
    struct Surface
    {
    private:
        friend class SurfaceCommandBuffer;
        SurfaceImpl* impl = nullptr;
    public:
        Surface() noexcept = default;
        explicit Surface(const SurfaceDesc& desc);

        ~Surface();

        Surface(const Surface&) = delete;
        Surface& operator=(const Surface&) = delete;

        Surface(Surface&&) noexcept;
        Surface& operator=(Surface&&) noexcept;

        void Create(const SurfaceDesc&);
        void Destroy() noexcept;
        [[nodiscard]] bool IsValid() const noexcept { return impl != nullptr; }

        auto CreateAnimation(const NaturalMotionAnimationDesc&) -> Animation;
        auto CreateGeometry(const ImplicitGeometryDesc&) -> Geometry;
        auto CreateBrush(const SolidColorBrushDesc&) -> Brush;
        auto CreateLayout(const LayoutDesc&) -> Layout;
    };

    struct SurfaceCommandBuffer
    {
    private:
        CommandBuffer* cmd;
        Surface* surfPtr;
    public:
        explicit SurfaceCommandBuffer(Surface& surface, CommandBuffer& commandBuffer) : surfPtr(&surface), cmd(&commandBuffer) {}

        void SetChange(Layout obj, LayoutProperties prop, float4 target, std::optional<Animation> animation);
        void SetChange(Geometry obj, ImplicitGeometryProperties prop, float4 target, std::optional<Animation> animation);
        void SetChange(Brush obj, SolidColorBrushProperties prop, float4 target, std::optional<Animation> animation);

        void Draw(uint32_t zOrder, Geometry geometry, Brush brush, Layout layout);
        void DrawSurface(const DrawSurfaceDesc&);
    };
};
#endif // LETTUCE_QUIMERA_API_HPP