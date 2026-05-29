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

using namespace Lettuce::Core;

namespace Lettuce::Quimera
{
    struct GeometryTag {};
    struct BrushTag {};
    struct LightTag {};
    struct AnimationTag {};

    using Geometry = Handle<GeometryTag>;
    using Brush = Handle<BrushTag>;
    using Light = Handle<LightTag>;
    using Animation = Handle<AnimationTag>;

    struct Color { float r, g, b, a; };
    struct Rect { float x, y, w, h; }

    struct SolidColorBrushDesc
    {
        float4 color;
    };

    struct ImplicitGeometryDesc
    {
        Rect bounds;
        float leftTopCornerRadious;
        float leftBottomCornerRadious;
        float rightTopCornerRadious;
        float rightBottomCornerRadious;
    };
    
    struct SurfaceDesc
    {
        Device& device;
        TextureView dstTexture;
        Rect renderArea;

        uint32_t maxImplicitGeometries;
        uint32_t maxBrushes;
        uint32_t maxDrawCommands;
    };

    struct SurfaceImpl;
    struct SurfaceCommandBuffer;
    struct Surface
    {
    private:
        friend class SurfaceCommandBuffer;
        SurfaceImpl* impl;
    public:
        void Create(const SurfaceDesc&);
        void Destroy();

        auto CreateGeometry(const ImplicitGeometryDesc&) -> Geometry;
        auto CreateBrush(const SolidColorBrushDesc&) -> Brush;

        void Draw(uint32_t zOrder, Geometry geometry, Brush brush, float3x3 transform);
    };

    struct SurfaceCommandBuffer
    {
    private:
        CommandBuffer* cmd;
        Surface* surfPtr;
    public:
        explicit SurfaceCommandBuffer(Surface& surface, CommandBuffer& commandBuffer) : surfPtr(&surface), cmd(&commandBuffer) {}

        void DrawSurface();
    };
};

#endif // LETTUCE_QUIMERA_API_HPP