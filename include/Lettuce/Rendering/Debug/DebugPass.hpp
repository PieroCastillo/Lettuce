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
    };

    struct DebugPassRecordDesc
    {
        GpuSpan<float3> positions;
        GpuSpan<uint32_t> indices;
        GpuSpan<ClusterStorage> clusters;
        GpuSpan<MeshStorage> meshes;
        uint32_t rtColorOutputIndex;
        uint32_t rtPickIndex;
    };

    class DebugPass
    {
    private:
        Device* m_device = nullptr;
        DescriptorTable dtPass;
        Pipeline pPass;
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