// standard headers
#include <algorithm>
#include <array>
#include <memory>
#include <memory_resource>
#include <ranges>
#include <span>
#include <vector>

// experimental headers

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/common.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Quimera/SurfaceImpl.hpp"
#include "Lettuce/Quimera/api.hpp"

using namespace Lettuce::Quimera;
using namespace Lettuce::Core;

void SurfaceCommandBuffer::SetChange(Layout obj, LayoutProperties prop, float4 target, std::optional<Animation> animation)
{
    throw NotImplemented("SetChange is not implemented yet.");
}

void SurfaceCommandBuffer::SetChange(Geometry obj, ImplicitGeometryProperties prop, float4 target, std::optional<Animation> animation)
{
    throw NotImplemented("SetChange is not implemented yet.");
}

void SurfaceCommandBuffer::SetChange(Brush obj, SolidColorBrushProperties prop, float4 target, std::optional<Animation> animation)
{
    throw NotImplemented("SetChange is not implemented yet.");
}

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
    constexpr auto epsilon = 1e-6f;

    auto surfImpl = surfPtr->impl;
    auto currentTime = std::chrono::steady_clock::now();

    // apply animations
    for (int i = 0; i < surfImpl->vAnimationInstances.size();)
    {
        auto& animInst = surfImpl->vAnimationInstances[i];
        const auto& anim = surfImpl->animations.get(animInst.srcAnim);
        auto dt = std::chrono::duration<float>(currentTime - animInst.lastUpdate).count();
        auto target = animInst.dstValue;

        if (anim.isNaturalMotion)
        {
            auto m = anim.mass;
            auto k = anim.stiffness;
            auto c = anim.dumping;
            auto energy = 0.0f;

            // integration :  implicit Euler - A stable
            // this assumes that buffers are fixed-capacity
            // for each component: ...
            for (auto j = 0; j < animInst.floatCount; ++j)
            {
                auto x = ((float*)animInst.heapAddr)[j];
                auto v = animInst.velocity[j];

                auto d = x - target[j];
                auto inv = 1.0f / (m + (c * dt) + (k * dt * dt));
                v = inv * ((m * v) - (k * dt * d));
                x += v * dt;

                ((float*)animInst.heapAddr)[j] = x;
                animInst.velocity[j] = v;

                energy += 0.5f * ((m * v * v) + (k * d * d));
            }

            // animation is finished if energy is so low
            // order does not matter, so we can do this
            if (energy <= epsilon)
            {
                std::swap(animInst, surfImpl->vAnimationInstances.back());
                surfImpl->vAnimationInstances.pop_back();
                continue;
            }

            animInst.lastUpdate = currentTime;
        }

        ++i;
    }

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
    *(surfImpl->surfaceData) = { static_cast<uint32_t>(desc.renderArea.w), static_cast<uint32_t>(desc.renderArea.h), drawCmdCount, (uint32_t)(surfImpl->bLayouts.maxCount / sizeof(LayoutStorage)) };

    auto allocs = std::array<PushAllocationBinding, 7>
    {
        surfImpl->surfaceData.getView(),
        surfImpl->vScratchTransforms.getView(),
        surfImpl->vScratchInvTransforms.getView(),
        surfImpl->bDrawCommands.mv,
        surfImpl->bLayouts.mv,
        surfImpl->bImplicitGeometry.mv,
        surfImpl->bSolidColorBrush.mv,
    };

    BarrierDesc bCompVert[] = { {
        .srcAccess = PipelineAccess::Write,
        .srcStage = PipelineStage::ComputeShader,
        .dstAccess = PipelineAccess::Read,
        .dstStage = PipelineStage::VertexShader,
    }, };

    AttachmentDesc colorAttachment[2] = { {
        .renderTarget = desc.dstTexture,
        .loadOp = LoadOp::Load,
    },
    {
        .renderTarget = desc.dstPickTexture,
        .loadOp = LoadOp::Load,
    }};
    AttachmentDesc depthAttachment = {
        .renderTarget = desc.dstDepthTexture,
        .loadOp = LoadOp::Clear,
    };

    RenderPassDesc renderPassDesc = {
        .width = desc.renderArea.w,
        .height = desc.renderArea.h,
        .colorAttachments = std::span(colorAttachment),
        .depthStencilAttachment = depthAttachment,
        .presentAttachmentIdx = 0,
    };

    // cmd->BindDescriptorTable(surfImpl->dtSurface, PipelineBindPoint::Compute);
    // cmd->BindPipeline(surfImpl->pPreprocess);
    // cmd->PushAllocations({ allocs, surfImpl->dtSurface });
    // cmd->Dispatch(((drawCmdCount + 31) / 32), 1, 1);

    // cmd->Barrier(bCompVert);

    cmd->BeginRendering(renderPassDesc);
    cmd->BindDescriptorTable(surfImpl->dtSurface, PipelineBindPoint::Graphics);
    cmd->BindPipeline(surfImpl->pRasterCommands);
    cmd->PushAllocations({ allocs, surfImpl->dtSurface });
    cmd->Draw(6, drawCmdCount);
    cmd->EndRendering();
}