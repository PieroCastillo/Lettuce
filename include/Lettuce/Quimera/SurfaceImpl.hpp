/*
Created by @PieroCastillo on 2026-05-29
*/
#ifndef LETTUCE_QUIMERA_SURFACE_IMPL_HPP
#define LETTUCE_QUIMERA_SURFACE_IMPL_HPP

// standard headers
#include <atomic>
#include <mutex>
#include <unordered_map>

// project headers
#include "../Core/api.hpp"
#include "../Core/ResourcePool.hpp"
#include "../Foundations/api.hpp"
#include "../Quimera/api.hpp"
#include "typesAccessData.hpp"
#include "typesStorage.hpp"

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
        std::mutex m_animationMutex;
        std::unordered_map<ChangeKey, float4, ChangeKeyHash> m_animationChanges;

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

        GpuMappedVector<DrawCommand> bDrawCommands;
        GpuPoolAllocator<LayoutStorage> bLayouts;
        GpuPoolAllocator<ImplicitGeometryStorage> bImplicitGeometry;
        GpuPoolAllocator<GlyphGeometryStorage> bGlyphGeometry;
        GpuPoolAllocator<SolidColorBrushStorage> bSolidColorBrush;

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