/*
Created by @PieroCastillo on 2026-11-15
*/
#ifndef LETTUCE_RENDERING_SHAPE_RENDER_PASS_HPP
#define LETTUCE_RENDERING_SHAPE_RENDER_PASS_HPP

#include "../../Core/api.hpp"
#include "../../Foundations/api.hpp"
#include "../BasePass.hpp"
#include "../types.hpp"
#include "../lightingTypes.hpp"

using namespace Lettuce::Foundations;

namespace Lettuce::Rendering::DeferredPlus
{
    struct ShapeRenderPassDesc
    {
        Device& device;
        DescriptorTable descriptorTable;
    };

    struct ShapeRenderPassRecordDesc
    {
        uint32_t fbWidth;
        uint32_t fbHeight;
        // input
        GpuSpan<SceneViewData> sceneViewData;
        GpuSpan<MaterialShadingData> materialShadingData;
        GpuSpan<MaterialPBRStorage> materialStorage;
        GpuSpan<SunLightData> sunLightData;
        GpuSpan<SpotLightData> spotLightData;
        GpuSpan<CascadeShadowMapData> cascadeShadowMapData;
        // output
        TextureView rtColorOutput; // FORMAT: RGBA32
        TextureView rtDepth; // FORMAT: Depth32
        TextureView rtMaterial; // FORMAT: Depth16
        TextureView rtPick; // FORMAT: Atomic_R32_UInt
    };

    class ShapeRenderPass : public BasePass
    {
    private:
        Pipeline pPass;
    public:
        ShapeRenderPass() noexcept = default;
        explicit ShapeRenderPass(const ShapeRenderPassDesc&);

        ~ShapeRenderPass();

        ShapeRenderPass(const ShapeRenderPass&) = delete;
        ShapeRenderPass& operator=(const ShapeRenderPass&) = delete;

        ShapeRenderPass(ShapeRenderPass&&) noexcept;
        ShapeRenderPass& operator=(ShapeRenderPass&&) noexcept;

        void Create(const ShapeRenderPassDesc&);
        void Destroy();

        void Record(CommandBuffer&, const ShapeRenderPassRecordDesc&);
    };
};
#endif // LETTUCE_RENDERING_SHAPE_RENDER_PASS_HPP