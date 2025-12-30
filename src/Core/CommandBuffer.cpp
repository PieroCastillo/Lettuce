// standard headers
#include <memory>
#include <array>
#include <print>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Core/CommandBufferImpl.hpp"
#include "Lettuce/Core/common.hpp"

using namespace Lettuce::Core;

void CommandBuffer::MemoryCopy(
    Allocation src,
    Allocation dst,
    uint64_t srcOffset,
    uint64_t dstOffset,
    uint64_t size
)
{

}

void CommandBuffer::BeginRendering(std::span<RenderTarget> renderTargets)
{
    // TODO: impl attachments
    VkRenderingInfo renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
    };
    vkCmdBeginRendering(impl->vkCmd, &renderingInfo);
}

void CommandBuffer::EndRendering()
{
    vkCmdEndRendering(impl->vkCmd);
}

void CommandBuffer::BindPipeline(Pipeline pipeline)
{
    auto pipelineInfo = impl->device->pipelines.get(pipeline);
    vkCmdBindPipeline(impl->vkCmd, pipelineInfo.bindPoint, pipelineInfo.pipeline);
}

void CommandBuffer::BindDescriptorTable(DescriptorTable descriptorTable, PipelineBindPoint bindPoint)
{
    auto& dt = impl->device->descriptorTables.get(descriptorTable);
    VkDescriptorBufferBindingInfoEXT bindingInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
        .address = dt.gpuAddress,
        .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT,
    };
    vkCmdBindDescriptorBuffersEXT(impl->vkCmd, 1, &bindingInfo);

    uint32_t bufferIdx = 0;
    uint64_t bufferOffset = 0;
    vkCmdSetDescriptorBufferOffsetsEXT(
        impl->vkCmd,
        ToVkPipelineBindPoint(bindPoint),
        dt.pipelineLayout,
        0,
        1,
        &bufferIdx,
        &bufferOffset);

    // push buffer addresses
    vkCmdPushConstants(impl->vkCmd, dt.pipelineLayout, VK_SHADER_STAGE_ALL, 0, dt.pushPayloadSize, dt.pushPayloadAddress);
}

void CommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount)
{
    vkCmdDraw(impl->vkCmd, vertexCount, instanceCount, 0, 0);
}

void CommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount)
{
    vkCmdDrawIndexed(impl->vkCmd, indexCount, instanceCount, 0, 0, 0);
}

void CommandBuffer::DrawMesh(uint32_t x, uint32_t y, uint32_t z)
{
    vkCmdDrawMeshTasksEXT(impl->vkCmd, x, y, z);
}

void CommandBuffer::ExecuteIndirect(IndirectSet indirectSet)
{
    // TODO: impl execute indirect
}

void CommandBuffer::Dispatch(uint32_t x, uint32_t y, uint32_t z)
{
    vkCmdDispatch(impl->vkCmd, x, y, z);
}

void CommandBuffer::Barrier(std::span<const BarrierDesc> barriers)
{
    uint32_t count = barriers.size();
    if (count == 0) [[unlikely]] return;

    VkMemoryBarrier2* memBarriers = (VkMemoryBarrier2*)alloca(count * sizeof(VkMemoryBarrier2));

    for (int i = 0; i < count; ++i)
    {
        const auto& barrier = barriers[i];
        memBarriers[i]  =
        {
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
            .srcStageMask = ToVkPipelineStageFlags(barrier.srcStage),
            .srcAccessMask = ToVkAccess(barrier.srcAccess),
            .dstStageMask = ToVkPipelineStageFlags(barrier.dstStage),
            .dstAccessMask = ToVkAccess(barrier.dstAccess),
        };
    }

    VkDependencyInfo  depInfo = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .memoryBarrierCount = count,
        .pMemoryBarriers = memBarriers,
    };
    vkCmdPipelineBarrier2(impl->vkCmd, &depInfo);
}