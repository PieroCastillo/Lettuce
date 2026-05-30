// standard headers
#include <memory>
#include <memory_resource>
#include <span>
#include <vector>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Quimera/SurfaceImpl.hpp"
#include "Lettuce/Quimera/api.hpp"

using namespace Lettuce::Quimera;
using namespace Lettuce::Core;

void SurfaceCommandBuffer::DrawSurface()
{
    auto surfImpl = surfPtr->impl;
    auto compBarrier = BarrierDesc {
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
    uint32_t tileXCount = (surfImpl->dstWidth + 15) / 16;
    uint32_t tileYCount = (surfImpl->dstHeight + 15) / 16;
    uint32_t threadGroupCount = surfImpl->device->QueryPreferredThreadCount();

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
    /* for Effects
    cmd->Barrier({compBarrier });
    cmd->BindPipeline(surfImpl->pEffects);
    cmd->PushAllocations({ allocs, surfImpl->dtSurface });
    cmd->Dispatch(tileXCount, tileYCount, 1);
    */
}