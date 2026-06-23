// standard headers
#include <algorithm>
#include <memory>
#include <memory_resource>
#include <ranges>
#include <span>
#include <vector>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Quimera/SurfaceImpl.hpp"
#include "Lettuce/Quimera/api.hpp"

using namespace Lettuce::Quimera;
using namespace Lettuce::Core;

void SurfaceCommandBuffer::Draw(uint32_t zOrder, Geometry geometry, Brush brush, float3x3 transform)
{
    surfPtr->impl->vTransforms.push_back(transform);
    uint32_t transformIdx = surfPtr->impl->vTransforms.size() - 1;

    surfPtr->impl->vDrawCommands.push_back(DrawCommand{ transformIdx, geometry.index, brush.index, zOrder, 0 });
}

void SurfaceCommandBuffer::DrawSurface(const DrawSurfaceDesc& desc)
{
    auto surfImpl = surfPtr->impl;

    // sort commands by depth
    struct
    {
        bool operator()(DrawCommand a, DrawCommand b) const { return a.zOrder < b.zOrder; }
    } customLess;
    auto& drawCmds = surfImpl->vDrawCommands;
    std::ranges::sort(drawCmds, customLess);

    // avoid UB
    if (drawCmds.size() >= surfImpl->bDrawCommands.maxCount || surfImpl->vTransforms.size() >= surfImpl->bTransforms.maxCount)
    {
        return;
    }

    auto compBarrier = BarrierDesc{
        PipelineAccess::Write, PipelineStage::ComputeShader,
        PipelineAccess::Read, PipelineStage::ComputeShader
    };

    auto allocs = std::array {
        surfImpl->bDrawCommands.mv,
        surfImpl->bTransforms.mv,
        surfImpl->bImplicitGeometries.mv,
        surfImpl->bBrushes.mv,
    };

    uint32_t cmdCount = surfImpl->bDrawCommands.offset + 1;
    uint32_t tileXCount = (desc.renderArea.w + 15) / 16;
    uint32_t tileYCount = (desc.renderArea.h + 15) / 16;
    uint32_t threadGroupCount = surfImpl->pDevice->QueryPreferredThreadCount();

    cmd->BindDescriptorTable(surfImpl->dtSurface, PipelineBindPoint::Compute);

    cmd->BindPipeline(surfImpl->pPrepare);
    cmd->PushAllocations({ allocs, surfImpl->dtSurface });
    cmd->Dispatch(ceil(cmdCount / threadGroupCount), 1, 1);

    cmd->Barrier({ compBarrier });

    cmd->BindPipeline(surfImpl->pTileBinning);
    cmd->PushAllocations({ allocs, surfImpl->dtSurface });
    cmd->Dispatch(ceil(cmdCount / threadGroupCount), 1, 1);

    cmd->Barrier({ compBarrier });

    cmd->BindPipeline(surfImpl->pBrushes);
    cmd->PushAllocations({ allocs, surfImpl->dtSurface });
    cmd->Dispatch(tileXCount, tileYCount, 1);
}