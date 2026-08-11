/*
Created by @PieroCastillo on 2026-05-17
*/
#ifndef LETTUCE_RENDERING_DEBUG_PASS_HPP
#define LETTUCE_RENDERING_DEBUG_PASS_HPP

#include "../../Core/api.hpp"
#include "../../Foundations/api.hpp"
#include "../types.hpp"

using namespace Lettuce::Foundations;

namespace Lettuce::Rendering::Debug
{
    struct DebugPassDesc
    {
        Device& device;
        DescriptorTable descriptorTable;
        uint32_t maxCulledInstances;
        Format colorOutputFormat;
    };

    struct DebugPassRecordDesc
    {
        uint32_t fbWidth;
        uint32_t fbHeight;
        // input
        GpuSpan<SceneViewData> sceneViewData;
        GpuSpan<float3> positions;
        GpuSpan<uint8_t> indices;
        GpuSpan<ClusterStorage> clusters;
        GpuSpan<MeshStorage> meshes;
        GpuSpan<InstanceStorage> culledInstances;
        // output
        TextureView rtColorOutput;
        TextureView rtDepth;
        TextureView rtPick; // FORMAT: Atomic_R32_UInt
    };

    class DebugPass
    {
    private:
        Device* m_device = nullptr;
        DescriptorTable dtPass;
        Pipeline pPass;
        Pipeline pBuildCommands;
        IndirectSet isPass;
        MemoryView mvIndirectDrawCommands;
    public:
        DebugPass() noexcept = default;
        explicit DebugPass(const DebugPassDesc&);

        ~DebugPass();

        DebugPass(const DebugPass&) = delete;
        DebugPass& operator=(const DebugPass&) = delete;

        DebugPass(DebugPass&&) noexcept;
        DebugPass& operator=(DebugPass&&) noexcept;

        void Create(const DebugPassDesc&);
        void Destroy();

        void Record(CommandBuffer&, const DebugPassRecordDesc&);
    };
};
#endif // LETTUCE_RENDERING_DEBUG_PASS_HPP