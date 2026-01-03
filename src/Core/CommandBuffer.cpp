// standard headers
#include <memory>
#include <array>
#include <print>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/common.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"

using namespace Lettuce::Core;

void CommandBuffer::MemoryCopy(
    const MemoryView& src,
    const MemoryView& dst,
    uint64_t srcOffset,
    uint64_t dstOffset,
    uint64_t size
)
{
    
}

void CommandBuffer::BeginRendering(const RenderPassDesc& desc)
{
    auto& renderTargets = impl.device->renderTargets;

    int colorCount = desc.colorAttachments.size();
    VkRenderingAttachmentInfo* colorAttachments = (VkRenderingAttachmentInfo*)alloca(sizeof(VkRenderingAttachmentInfo) * colorCount);

    for (int i = 0; i < colorCount; ++i)
    {
        const auto& attachment = desc.colorAttachments[i];
        const auto& rt = renderTargets.get(attachment.renderTarget);

        colorAttachments[i] = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = rt.imageView,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
            .loadOp = ToVkAttachmentLoadOp(attachment.loadOp),
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = rt.defaultClearValue,
        };
    }

    VkRenderingInfo renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {{0,0},{ desc.width, desc.height }},
        .layerCount = 1,
        .viewMask = 0,
        .colorAttachmentCount = (uint32_t)colorCount,
        .pColorAttachments = colorAttachments,
    };

    VkRenderingAttachmentInfo attachmentInfo;
    if (desc.depthStencilAttachment)
    {
        const auto& rt = renderTargets.get(desc.depthStencilAttachment->renderTarget);

        attachmentInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = rt.imageView,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
            .loadOp = ToVkAttachmentLoadOp(desc.depthStencilAttachment->loadOp),
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = rt.defaultClearValue,
        };
        renderingInfo.pDepthAttachment = &attachmentInfo;
        renderingInfo.pStencilAttachment = &attachmentInfo;
    }

    vkCmdBeginRendering((VkCommandBuffer)impl.handle, &renderingInfo);
}

void CommandBuffer::EndRendering()
{
    vkCmdEndRendering((VkCommandBuffer)impl.handle);
}

void CommandBuffer::BindPipeline(Pipeline pipeline)
{
    auto pipelineInfo = impl.device->pipelines.get(pipeline);
    vkCmdBindPipeline((VkCommandBuffer)impl.handle, pipelineInfo.bindPoint, pipelineInfo.pipeline);
}

void CommandBuffer::BindDescriptorTable(DescriptorTable descriptorTable, PipelineBindPoint bindPoint)
{
    auto& dt = impl.device->descriptorTables.get(descriptorTable);
    VkDescriptorBufferBindingInfoEXT bindingInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
        .address = dt.gpuAddress,
        .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT,
    };
    vkCmdBindDescriptorBuffersEXT((VkCommandBuffer)impl.handle, 1, &bindingInfo);

    uint32_t bufferIdx = 0;
    uint64_t bufferOffset = 0;
    vkCmdSetDescriptorBufferOffsetsEXT(
        (VkCommandBuffer)impl.handle,
        ToVkPipelineBindPoint(bindPoint),
        dt.pipelineLayout,
        0,
        1,
        &bufferIdx,
        &bufferOffset);

    // push buffer addresses
    vkCmdPushConstants((VkCommandBuffer)impl.handle, dt.pipelineLayout, VK_SHADER_STAGE_ALL, 0, dt.pushPayloadSize, dt.pushPayloadAddress);
}

void CommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount)
{
    vkCmdDraw((VkCommandBuffer)impl.handle, vertexCount, instanceCount, 0, 0);
}

void CommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount)
{
    vkCmdDrawIndexed((VkCommandBuffer)impl.handle, indexCount, instanceCount, 0, 0, 0);
}

void CommandBuffer::DrawMesh(uint32_t x, uint32_t y, uint32_t z)
{
    vkCmdDrawMeshTasksEXT((VkCommandBuffer)impl.handle, x, y, z);
}

void CommandBuffer::ExecuteIndirect(IndirectSet indirectSet)
{
    // TODO: impl execute indirect
}

void CommandBuffer::Dispatch(uint32_t x, uint32_t y, uint32_t z)
{
    vkCmdDispatch((VkCommandBuffer)impl.handle, x, y, z);
}

void CommandBuffer::Barrier(std::span<const BarrierDesc> barriers)
{
    uint32_t count = barriers.size();
    if (count == 0) [[unlikely]] return;

    VkMemoryBarrier2* memBarriers = (VkMemoryBarrier2*)alloca(count * sizeof(VkMemoryBarrier2));

    for (int i = 0; i < count; ++i)
    {
        const auto& barrier = barriers[i];
        memBarriers[i] =
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
    vkCmdPipelineBarrier2((VkCommandBuffer)impl.handle, &depInfo);
}