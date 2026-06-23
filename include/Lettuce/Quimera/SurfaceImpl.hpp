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
    struct ImplicitGeometryStorage
    {
        float x, y, w, h;
        float ctl, ctr, cbl, cbr; // corners
    };

    struct BrushStorage
    {
        uint32_t b;
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
        uint32_t zOrder;
        uint32_t flags;
    };

    template<typename T>
    struct Buffer
    {
        MemoryView mv;
        HostAddress addr;
        uint32_t offset;
        uint32_t maxCount;
        static constexpr uint32_t elementSize = sizeof(T);
    };

    struct SurfaceImpl
    {
        Device* pDevice = nullptr;
    
        DescriptorTable dtSurface;

        Pipeline pPrepare;
        Pipeline pTileBinning;
        Pipeline pBrushes;
        Pipeline pEffects;

        Buffer<DrawCommand> bDrawCommands;
        Buffer<float3x3> bTransforms;
        Buffer<ImplicitGeometryStorage> bImplicitGeometries;
        Buffer<BrushStorage> bBrushes;

        std::vector<DrawCommand> vDrawCommands;
        std::vector<float3x3> vTransforms;

        void Create(const SurfaceDesc&);
        void Destroy();
    };
};
#endif // LETTUCE_QUIMERA_SURFACE_IMPL_HPP