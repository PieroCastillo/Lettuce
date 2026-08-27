/*
Created by @PieroCastillo on 2026-05-29
*/
#ifndef LETTUCE_QUIMERA_SURFACE_IMPL_HPP
#define LETTUCE_QUIMERA_SURFACE_IMPL_HPP

// standard headers
#include <atomic>
#include <unordered_map>

// project headers
#include "../Core/api.hpp"
#include "../Core/ResourcePool.hpp"
#include "../Foundations/api.hpp"
#include "../Quimera/api.hpp"

// external headers
#include "freetype/freetype.h"
#include "harfbuzz/hb.h"

using namespace Lettuce::Foundations;

namespace Lettuce::Quimera
{
    enum class GeometryHeap : uint32_t
    {
        Implicit = 0,
        Font = 1,
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
        uint32_t layoutCount;
    };

    struct GeometryAccessData
    {
        uint32_t geometryIdx;
        uint32_t geometryHeapIdx;
    };

    struct GlyphAccessData
    {
        TextureView texture;
        uint32_t descriptorIdx;
        uint32_t storageIdx;
    };

    struct FontAccessData
    {
        std::unique_ptr<uint8_t[]> fontData;
        FT_Face fontFace;
        hb_font_t* hbFont;
        // key: glyphID, data: TextureView | DescriptorIdx
        std::unordered_map<uint32_t, GlyphAccessData> glyphIdxDataMap;
    };

    struct BrushAccessData
    {
        uint32_t brushIdx;
        uint32_t brushHeapIdx;
    };

    struct LayoutAccessData
    {
        uint32_t layoutIdx;
    };

    struct ImplicitGeometryStorage
    {
        float ctl, ctr, cbl, cbr; // corners
    };

    struct GlyphGeometryStorage
    {
        uint32_t descriptorIdx;
        float bitmapTop;
        float bitmapLeft;
    };

    struct SolidColorBrushStorage
    {
        float4 color;
    };

    struct LayoutStorage
    {
        float2 position;
        float2 scale;
        float2 skew;
        float2 anchorPoint;
        float rotation;
    };

    struct DrawCommand
    {
        uint32_t layoutIdx;
        uint32_t geometryIdx;
        uint32_t brushIdx;
        uint32_t zOrder;
        // [ 1 bit (clipped) | 1 bit (ignoreTransform) | ... | 4 bit (effect heap) | 4 bit (brush heap) | 4 bit (geometry heap)]
        uint32_t flags;
        uint32_t reserved1;
        uint32_t reserved2;
        uint32_t reserved3;
    };

    struct AnimationData
    {
        bool isNaturalMotion;

        // natural motion
        float mass;
        float stiffness;
        float dumping;
    };

    struct AnimationInstance
    {
        Animation srcAnim;
        HostAddress heapAddr;
        uint32_t floatCount;
        float4 srcValue;
        float4 dstValue;
        float4 velocity;
        std::chrono::steady_clock::time_point lastUpdate;
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

    struct TextureRegistry
    {
        std::vector<TextureView> slots;
        std::vector<uint32_t> freeIndices;

        auto Push(TextureView texture) -> uint32_t
        {
            uint32_t index;

            if (!freeIndices.empty())
            {
                index = freeIndices.back();
                freeIndices.pop_back();

                slots[index] = texture;
            }
            else
            {
                index = slots.size();
                slots.push_back(texture);
            }

            return index;
        }

        void Remove(uint32_t index)
        {
            slots[index] = {};
            freeIndices.push_back(index);
        }
    };

    struct SurfaceImpl
    {
        Device* pDevice = nullptr;
        std::chrono::steady_clock::time_point m_startTime;

        DescriptorTable dtSurface;
        TextureRegistry sampledImgRegistry;

        Pipeline pPreprocess;
        Pipeline pRasterCommands;

        ResourcePool<Geometry, GeometryAccessData> geometries;
        ResourcePool<Font, FontAccessData> fonts;
        ResourcePool<Brush, BrushAccessData> brushes;
        ResourcePool<Layout, LayoutAccessData> layouts;
        ResourcePool<Animation, AnimationData> animations;

        GpuUniquePtr<SurfaceData> surfaceData;

        GpuStorageVector<float3x3> vScratchTransforms;
        GpuStorageVector<float3x3> vScratchInvTransforms;

        Buffer<DrawCommand> bDrawCommands;
        Buffer<LayoutStorage> bLayouts;
        Buffer<ImplicitGeometryStorage> bImplicitGeometry;
        Buffer<GlyphGeometryStorage> bGlyphGeometry;
        Buffer<SolidColorBrushStorage> bSolidColorBrush;

        std::vector<AnimationInstance> vAnimationInstances;
        std::vector<DrawCommand> vDrawCommands;

        Sampler samplerDefault;
        Sampler samplerSdf;
        TextureView twLastRenderTarget;

        // font manager
        FT_Library fontLib;

        void Create(const SurfaceDesc&);
        void Destroy();
    };
};
#endif // LETTUCE_QUIMERA_SURFACE_IMPL_HPP