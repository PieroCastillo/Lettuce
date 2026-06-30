// standard headers
#include <algorithm>
#include <array>
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

void SurfaceCommandBuffer::Draw(uint32_t zOrder, Geometry geometry, Brush brush, Layout layout)
{
    auto surfImpl = surfPtr->impl;

    const auto& geoInfo = surfImpl->geometries.get(geometry);
    const auto& brushInfo = surfImpl->brushes.get(brush);
    const auto& layoutInfo = surfImpl->layouts.get(layout);

    auto flags = DrawCommandPackFlags(geoInfo.geometryHeapIdx, brushInfo.brushHeapIdx, 0, false, false);

    surfImpl->vDrawCommands.push_back(DrawCommand{ layoutInfo.layoutIdx, geoInfo.geometryIdx, brushInfo.brushIdx, zOrder, false });
}

void SurfaceCommandBuffer::DrawSurface(const DrawSurfaceDesc& desc)
{
    auto surfImpl = surfPtr->impl;

    // sort commands by depth
    auto& drawCmds = surfImpl->vDrawCommands;
    std::ranges::stable_sort(drawCmds, std::ranges::greater{}, &DrawCommand::zOrder);
    auto drawCmdCount = (uint32_t)drawCmds.size();

    // avoid UB
    if (drawCmds.size() >= surfImpl->bDrawCommands.maxCount)
        return;

    // copy commands
    memcpy(surfImpl->bDrawCommands.addr, drawCmds.data(), drawCmds.size() * sizeof(DrawCommand));

    // clear immediate render info for the next frame
    surfImpl->vDrawCommands.clear();

    // set render target
    if (surfImpl->twLastRenderTarget != desc.dstTexture) [[likely]]
    {
        auto stgTextures = std::array{
            std::make_pair(0u, desc.dstTexture),
        };
        PushResourceDescriptorsDesc pushResDesc = {
            .storageTextures = std::span(stgTextures),
            .descriptorTable = surfImpl->dtSurface,
        };
        surfImpl->pDevice->PushResourceDescriptors(pushResDesc);

        surfImpl->twLastRenderTarget = desc.dstTexture;
    }

    // set width & height
    *(surfImpl->mvSurfaceDataPtr) = { static_cast<uint32_t>(desc.renderArea.w), static_cast<uint32_t>(desc.renderArea.h), drawCmdCount };

    auto allocs = std::array{
        surfImpl->mvSurfaceData,
        surfImpl->mvScratchTransforms,
        surfImpl->bDrawCommands.mv,
        surfImpl->bLayouts.mv,
        surfImpl->bImplicitGeometry.mv,
        surfImpl->bSolidColorBrush.mv,
    };

    uint32_t cmdCount = surfImpl->bDrawCommands.offset + 1;
    uint32_t tileXCount = (desc.renderArea.w + 15) / 16;
    uint32_t tileYCount = (desc.renderArea.h + 15) / 16;

    cmd->BindDescriptorTable(surfImpl->dtSurface, PipelineBindPoint::Compute);
    cmd->BindPipeline(surfImpl->pDrawCommands);
    cmd->PushAllocations({ allocs, surfImpl->dtSurface });
    cmd->Dispatch(tileXCount, tileYCount, 1);
}