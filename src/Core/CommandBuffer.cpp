// standard headers
#include <algorithm>
#include <array>
#include <format>
#include <memory>
#include <print>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/common.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"

using namespace Lettuce::Core;

void CommandBuffer::ClearTexture(const ClearTextureDesc& desc)
{
    auto& dev = impl.device;
    auto& imgInfo = dev->textures.get(desc.texture);

    auto color = VkClearColorValue{};
    color.float32[0] = desc.color.value[0];
    color.float32[1] = desc.color.value[1];
    color.float32[2] = desc.color.value[2];
    color.float32[3] = desc.color.value[3];

    auto range = VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, desc.baseLevel, desc.levelCount, desc.baseLayer, desc.layerCount };

    vkCmdClearColorImage((VkCommandBuffer)impl.handle, imgInfo.image, VK_IMAGE_LAYOUT_GENERAL, &color, 1, &range);
}

void CommandBuffer::BeginRendering(const RenderPassDesc& desc)
{
    int colorCount = desc.colorAttachments.size();
    VkRenderingAttachmentInfo* colorAttachments = (VkRenderingAttachmentInfo*)alloca(sizeof(VkRenderingAttachmentInfo) * colorCount);

    for (int i = 0; i < colorCount; ++i)
    {
        const auto& attachment = desc.colorAttachments[i];
        const auto& rt = impl.device->textures.get(attachment.renderTarget);

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
        const auto& rt = impl.device->textures.get(desc.depthStencilAttachment->renderTarget);

        attachmentInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = rt.imageView,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
            .loadOp = ToVkAttachmentLoadOp(desc.depthStencilAttachment->loadOp),
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = rt.defaultClearValue,
        };
        renderingInfo.pDepthAttachment = &attachmentInfo;
        // renderingInfo.pStencilAttachment = &attachmentInfo;
    }

    impl.currentPresentTarget = std::nullopt;
    if(desc.presentAttachmentIdx && desc.presentAttachmentIdx.value() < desc.colorAttachments.size())
    {
        impl.currentPresentTarget = desc.colorAttachments[desc.presentAttachmentIdx.value()].renderTarget;

        VkImageMemoryBarrier2 imgBar = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .srcAccessMask =  0,
            .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_GENERAL,
            .image = impl.device->textures.get(impl.currentPresentTarget.value()).image,
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0,1,0,1},
        };

        VkDependencyInfo depInfo = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &imgBar,
        };
        vkCmdPipelineBarrier2((VkCommandBuffer)impl.handle, &depInfo);
    }

    auto cmd = (VkCommandBuffer)impl.handle;
    vkCmdBeginRendering(cmd, &renderingInfo);

    VkViewport vw = { 0, 0, static_cast<uint32_t>(desc.width), static_cast<uint32_t>(desc.height), 0, 1 };
    VkRect2D scissor = { { 0,0 }, { desc.width, desc.height } };
    vkCmdSetViewport(cmd, 0, 1, &vw);
    vkCmdSetScissor(cmd, 0, 1, &scissor);
}

void CommandBuffer::EndRendering()
{
    vkCmdEndRendering((VkCommandBuffer)impl.handle);

    if (impl.currentPresentTarget)
    {
        VkImageMemoryBarrier2 imgBar = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .image = impl.device->textures.get(impl.currentPresentTarget.value()).image,
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0,1,0,1},
        };

        VkDependencyInfo depInfo = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &imgBar,
        };
        vkCmdPipelineBarrier2((VkCommandBuffer)impl.handle, &depInfo);
    }
    impl.currentPresentTarget = std::nullopt;
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
}

void CommandBuffer::PushAllocations(const PushAllocationsDesc& desc)
{
    auto& dt = impl.device->descriptorTables.get(desc.descriptorTable);
    auto payloadSize = std::min<uint32_t>(impl.device->props.maxPushAllocationsCount, desc.allocations.size()) * sizeof(uint64_t);
    auto count = payloadSize / sizeof(uint64_t);
    auto data = (uint64_t*)alloca(payloadSize);

    int idx = 0;
    for (const auto& memView : desc.allocations)
    {
        data[idx] = impl.device->memories.get(memView).gpuAddress;
        ++idx;
    }

    vkCmdPushConstants((VkCommandBuffer)impl.handle, dt.pipelineLayout, VK_SHADER_STAGE_ALL, 0, payloadSize, data);
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

void CommandBuffer::ExecuteIndirect(const ExecuteIndirectDesc& desc)
{
    // TODO: impl execute indirect
    auto& set = impl.device->indirectSets.get(desc.indirectSet);
    auto cmd = (VkCommandBuffer)impl.handle;
    auto buffer = set.indirectSetBuffer;
    auto offset = std::max<uint32_t>(0, desc.offset);

    switch (set.type)
    {
    case IndirectType::Draw:
        vkCmdDrawIndirectCount(cmd, buffer, 4 + offset, buffer, 0, desc.maxDrawCount, set.stride);
        break;
    case IndirectType::DrawIndexed:
        vkCmdDrawIndexedIndirectCount(cmd, buffer, 4 + offset, buffer, 0, desc.maxDrawCount, set.stride);
        break;
    case IndirectType::DrawMesh:
        vkCmdDrawMeshTasksIndirectCountEXT(cmd, buffer, 4 + offset, buffer, 0, desc.maxDrawCount, set.stride);
        break;
    case IndirectType::Dispatch:
        vkCmdDispatchIndirect(cmd, buffer, 4 + offset);
        break;
    default:
        break;
    }
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
/*
auto CommandBuffer::PrepareTexture(Texture texture)
{
    auto texInfo = impl.device->textures.get(texture);
    auto subRes = VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, texInfo.mipCount, 0, texInfo.layerCount };
    auto img = texInfo.image;

    VkImageMemoryBarrier2 barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_GENERAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = img,
        .subresourceRange = subRes,
    };
    VkDependencyInfo  depInfo = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier,
    };
    vkCmdPipelineBarrier2((VkCommandBuffer)impl.handle, &depInfo);
}

*/
void CommandBuffer::ResetCount(IndirectSet indirectSet)
{
    vkCmdFillBuffer((VkCommandBuffer)impl.handle, impl.device->indirectSets.get(indirectSet).indirectSetBuffer, 0, sizeof(uint32_t), 0);
}