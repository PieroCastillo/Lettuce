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

void CommandBuffer::MemoryCopy(const MemoryToMemoryCopy& copy)
{
    VkBufferCopy bufferCopy = {
        .srcOffset = copy.srcMemory.offset,
        .dstOffset = copy.dstMemory.offset,
        .size = copy.size,
    };

    auto& buffers = impl.device->buffers;
    vkCmdCopyBuffer(
        (VkCommandBuffer)impl.handle,
        buffers.get(copy.srcMemory.buffer).buffer,
        buffers.get(copy.dstMemory.buffer).buffer,
        1, &bufferCopy);
}

void CommandBuffer::MemoryCopy(const MemoryToTextureCopy& copy)
{
    auto& dev = impl.device;
    auto& imgInfo = dev->textures.get(copy.dstTexture);

    VkBufferImageCopy imageCopy = {
        .bufferOffset = copy.srcMemory.offset,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, copy.mipmapLevel, copy.layerBaseLevel, copy.layerCount},
        .imageOffset = {0,0,0},
        .imageExtent = {imgInfo.width, imgInfo.height, 1 },
    };

    vkCmdCopyBufferToImage(
        (VkCommandBuffer)impl.handle,
        dev->buffers.get(copy.srcMemory.buffer).buffer,
        imgInfo.image,
        VK_IMAGE_LAYOUT_GENERAL, 1, &imageCopy);
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

    auto cmd = (VkCommandBuffer)impl.handle;
    vkCmdBeginRendering(cmd, &renderingInfo);

    VkViewport vw = { 0, 0, desc.width, desc.height, 0, 1 };
    VkRect2D scissor = { { 0,0 }, { desc.width, desc.height } };
    vkCmdSetViewport(cmd, 0, 1, &vw);
    vkCmdSetScissor(cmd, 0, 1, &scissor);
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
    // vkCmdPushConstants((VkCommandBuffer)impl.handle, dt.pipelineLayout, VK_SHADER_STAGE_ALL, 0, dt.pushPayloadSize, dt.pushPayloadAddress);
}

void CommandBuffer::PushAllocations(const PushAllocationsDesc& desc)
{
    auto& dt = impl.device->descriptorTables.get(desc.descriptorTable);
    auto payloadSize =  impl.device->props.maxPushAllocationsCount * sizeof(uint64_t);
    uint64_t* data = (uint64_t*)alloca(payloadSize);

    for(auto const& [idx, memView] : desc.allocations)
    {
        data[idx] = memView.gpuAddress;
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

void CommandBuffer::PrepareTexture(Texture texture)
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