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
    struct GeometryPassDesc
    {
        Device& device;
        DescriptorTable descriptorTable;
        uint32_t maxCulledInstances;
    };

    struct GeometryPassRecordDesc
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
        TextureView rtColorOutput; // FORMAT: RGBA32
        TextureView rtDepth; // FORMAT: Depth32
        TextureView rtMaterial; // FORMAT: R16
        TextureView rtPick; // FORMAT: Atomic_R32_UInt
    };

    class GeometryPass
    {
    private:
        Device* m_device = nullptr;
        DescriptorTable dtPass;
        Pipeline pPass;
        Pipeline pBuildCommands;
        IndirectSet isPass;
        MemoryView mvIndirectDrawCommands;
    public:
        GeometryPass() noexcept = default;
        explicit GeometryPass(const GeometryPassDesc&);

        ~GeometryPass();

        GeometryPass(const GeometryPass&) = delete;
        GeometryPass& operator=(const GeometryPass&) = delete;

        GeometryPass(GeometryPass&&) noexcept;
        GeometryPass& operator=(GeometryPass&&) noexcept;

        void Create(const GeometryPassDesc&);
        void Destroy();

        void Record(CommandBuffer&, const GeometryPassRecordDesc&);
    };
};
#endif // LETTUCE_RENDERING_DEBUG_PASS_HPP