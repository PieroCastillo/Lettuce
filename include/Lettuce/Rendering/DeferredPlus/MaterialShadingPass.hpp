/*
Created by @PieroCastillo on 2026-05-17
*/
#ifndef LETTUCE_RENDERING_DEBUG_PASS_HPP
#define LETTUCE_RENDERING_DEBUG_PASS_HPP

#include "../../Core/api.hpp"
#include "../../Foundations/api.hpp"
#include "../BasePass.hpp"
#include "../types.hpp"

using namespace Lettuce::Foundations;

namespace Lettuce::Rendering::DeferredPlus
{
    struct MaterialShadingPassDesc
    {
        Device& device;
        DescriptorTable descriptorTable;
        uint32_t maxCulledInstances;
    };

    struct MaterialShadingPassRecordDesc
    {
        uint32_t fbWidth;
        uint32_t fbHeight;
        // input
        GpuSpan<SceneViewData> sceneViewData;
        // output
        TextureView rtColorOutput; // FORMAT: RGBA32
        TextureView rtDepth; // FORMAT: Depth32
        TextureView rtMaterial; // FORMAT: Depth16
        TextureView rtPick; // FORMAT: Atomic_R32_UInt
    };

    class MaterialShadingPass : public BasePass
    {
    private:
        Pipeline pPass;
    public:
        MaterialShadingPass() noexcept = default;
        explicit MaterialShadingPass(const MaterialShadingPassDesc&);

        ~MaterialShadingPass();

        MaterialShadingPass(const MaterialShadingPass&) = delete;
        MaterialShadingPass& operator=(const MaterialShadingPass&) = delete;

        MaterialShadingPass(MaterialShadingPass&&) noexcept;
        MaterialShadingPass& operator=(MaterialShadingPass&&) noexcept;

        void Create(const MaterialShadingPassDesc&);
        void Destroy();

        void Record(CommandBuffer&, const MaterialShadingPassRecordDesc&);
    };
};
#endif // LETTUCE_RENDERING_DEBUG_PASS_HPP