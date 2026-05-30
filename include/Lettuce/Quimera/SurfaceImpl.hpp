/*
Created by @PieroCastillo on 2026-05-29
*/
#ifndef LETTUCE_QUIMERA_SURFACE_IMPL_HPP
#define LETTUCE_QUIMERA_SURFACE_IMPL_HPP

// project headers
#include "../Core/api.hpp"
#include "../Core/ResourcePool.hpp"
#include "../Quimera/api.hpp"

namespace Lettuce::Quimera
{
    struct ImplicitGeometry
    {
        float x, y, w, h;
        float ctl, ctr, cbl, cbr; // corners
    };

    constexpr uint32_t Draw_GeometryParam = 1 << 0;
    constexpr uint32_t Draw_GeometryPath = 1 << 1;
    constexpr uint32_t Draw_GeometryAtlas = 1 << 2;
    constexpr uint32_t Draw_TransformIgnore = 1 << 4;

    struct DrawCommand
    {
        uint32_t transformIdx;
        uint32_t geometryIdx;
        uint32_t brushIdx;
        uint32_t flags;
    };

    struct SurfaceImpl
    {
        Device* device = nullptr;
        TextureView dstTexture;
        DescriptorTable dtSurface;

        Pipeline pPrepare;
        Pipeline pTileBinning;
        Pipeline pBrushes;
        Pipeline pEffects;

        MemoryView mvDrawCommands;
        MemoryView mvTransforms;
        MemoryView mvImplicitGeometries;
        MemoryView mvBrushes;

        void Create(const SurfaceDesc&);
        void Destroy();
    };
};

#endif // LETTUCE_QUIMERA_SURFACE_IMPL_HPP