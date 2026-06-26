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
    enum class GeometryHeap : uint32_t
    {
        Implicit = 0,
    };

    enum class BrushHeap : uint32_t
    {
        SolidColor = 0,
    };

    struct SurfaceData
    {
        uint32_t width;
        uint32_t height;
        uint32_t drawCommandCount;
        uint32_t reserved1;
    };

    struct GeometryAccessData
    {
        uint32_t geometryIdx;
        uint32_t geometryHeapIdx;
    };

    struct BrushAccessData
    {
        uint32_t brushIdx;
        uint32_t brushHeapIdx;
    };

    struct ImplicitGeometryStorage
    {
        float w, h;
        float ctl, ctr, cbl, cbr; // corners
    };

    struct SolidColorBrushStorage
    {
        float4 color;
    };

    struct DrawCommand
    {
        uint32_t transformIdx;
        uint32_t geometryIdx;
        uint32_t brushIdx;
        uint32_t zOrder;
        // [ 1 bit (clipped) | 1 bit (ignoreTransform) | ... | 4 bit (effect heap) | 4 bit (brush heap) | 4 bit (geometry heap)]
        uint32_t flags;
        uint32_t reserved1;
        uint32_t reserved2;
        uint32_t reserved3;
    };

    inline uint32_t DrawCommandPackFlags(uint32_t geometryHeap, uint32_t brushHeap, uint32_t effectHeap, bool ignoreTransform, bool clipped)
    {
        return (geometryHeap & 0xF) |
            ((brushHeap & 0xF) << 4) |
            ((effectHeap & 0xF) << 8) |
            (uint32_t(ignoreTransform) << 30) |
            (uint32_t(clipped) << 31);
    }

    template<typename T>
    struct Buffer
    {
        MemoryView mv;
        HostAddress addr;
        uint32_t offset;
        uint32_t maxCount;
        std::vector<uint32_t> freeIndices;

        static constexpr uint32_t elementSize = sizeof(T);

        Buffer() = default;

        explicit Buffer(Device* pDevice, uint32_t count) : offset(0), maxCount(count)
        {
            mv = pDevice->CreateMemoryView({ maxCount * sizeof(T), true });
            addr = pDevice->GetMemoryViewInfo(mv).cpuAddress;
        }

        uint32_t Push(const T& value)
        {
            uint32_t index;

            if (!freeIndices.empty())
            {
                index = freeIndices.back();
                freeIndices.pop_back();
            }
            else
            {
                index = offset++;
            }

            reinterpret_cast<T*>(addr)[index] = value;
            return index;
        }

        void Free(uint32_t index)
        {
            freeIndices.push_back(index);
        }
    };

    struct SurfaceImpl
    {
        Device* pDevice = nullptr;

        DescriptorTable dtSurface;
        Pipeline pDrawCommands;

        ResourcePool<Geometry, GeometryAccessData> geometries;
        ResourcePool<Brush, BrushAccessData> brushes;

        MemoryView mvSurfaceData;
        SurfaceData* mvSurfaceDataPtr;

        Buffer<DrawCommand> bDrawCommands;
        Buffer<float3x3> bTransforms;
        Buffer<ImplicitGeometryStorage> bImplicitGeometry;
        Buffer<SolidColorBrushStorage> bSolidColorBrush;

        std::vector<DrawCommand> vDrawCommands;
        std::vector<float3x3> vTransforms;

        TextureView twLastRenderTarget;

        void Create(const SurfaceDesc&);
        void Destroy();
    };
};
#endif // LETTUCE_QUIMERA_SURFACE_IMPL_HPP