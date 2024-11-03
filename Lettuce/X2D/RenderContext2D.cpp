//
// Created by piero on 14/10/2024.
//
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Lettuce/Core/common.hpp"
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/X2D/RenderContext2D.hpp"

using namespace Lettuce::X2D;

// use subpasses
//  use secondary command buffers and record with multi threading

RenderContext2D::RenderContext2D(Device device, VkFormat swapchainImageFormat)
{
    // swapchain color attachment
    renderPass.AddAttachment(0, AttachmentType::Color,
                             swapchainImageFormat,
                             LoadOp::DontCare, StoreOp::Store,
                             LoadOp::None, StoreOp::None,
                             ImageLayout::ColorAttachmentOptimal, ImageLayout::PresentSrc, ImageLayout::ColorAttachmentOptimal);
    // // color attachment
    // renderPass.AddAttachment(1, AttachmentType::Color,
    //                          swapchainImageFormat,
    //                          LoadOp::DontCare, StoreOp::Store,
    //                          LoadOp::None, StoreOp::None,
    //                          ImageLayout::ColorAttachmentOptimal, ImageLayout::ReadOnlyOptimal, ImageLayout::ColorAttachmentOptimal);

    // // depth attachment
    // renderPass.AddAttachment(2, AttachmentType::DepthStencil,
    //                          device._gpu.GetDepthFormat(),
    //                          LoadOp::DontCare, StoreOp::Store,
    //                          LoadOp::DontCare, StoreOp::Store,
    //                          ImageLayout::DepthStencilAttachmentOptimal, ImageLayout::DepthAttachmentStencilReadOnlyOptimal, ImageLayout::DepthStencilAttachmentOptimal);

    renderPass.AddSubpass(0, BindPoint::Graphics, {0});
    // renderPass.AddSubpass(1, BindPoint::Graphics, {0,1,2});

    // renderPass.AddDependency(0, VK_SUBPASS_EXTERNAL, AccessStage::)

    renderPass.Build(device);
}
void RenderContext2D::Record(VkCommandBuffer cmd, VkImage swapchainImage, uint32_t swapchainImageIndex, VkClearColorValue color)
{
    // rendering cmd

    VkImageSubresourceRange imgSubresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    checkResult(vkResetCommandBuffer(cmd, 0));
    //    VkClearValue clearValues[2];
    //    clearValues[0].color = {{0.5f, 0.5f, 0.5f, 1.0f}};
    //    clearValues[1].depthStencil = {1, 0};
    VkClearValue clearValue;
    clearValue.color = color;

    VkCommandBufferBeginInfo cmdBI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
    };
    checkResult(vkBeginCommandBuffer(cmd, &cmdBI));

    VkImageMemoryBarrier2 imageBarrier2 = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = swapchainImage,
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };

    VkDependencyInfo dependencyI = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &imageBarrier2,
    };

    vkCmdPipelineBarrier2(cmd, &dependencyI);

    VkRect2D renderArea;
    renderArea.extent.height = height;
    renderArea.extent.width = width;
    renderArea.offset.x = 0;
    renderArea.offset.y = 0;

    VkRenderPassBeginInfo renderPassBI = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass._renderPass,
        .framebuffer = renderPass._framebuffers[(int)swapchainImageIndex], //be careful! this index can change in the future
        .renderArea = renderArea,
        .clearValueCount = 1,
        .pClearValues = &clearValue,
    };

    vkCmdBeginRenderPass(cmd, &renderPassBI, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

    for(auto [geometryPtr, materialPtr] : materialsPairs)
    {
        renderMaterialPair(cmd, geometryPtr, materialPtr);
    }
    //next subpass
    //render lights
    //next subpass
    //render effects

    vkCmdEndRenderPass(cmd);

    checkResult(vkEndCommandBuffer(cmd));
}

void RenderContext2D::renderMaterialPair(VkCommandBuffer cmd, std::shared_ptr<Geometries::GeometryBase> geometryPtr, std::shared_ptr<Materials::MaterialBase> materialPtr)
{
    // link buffers, descriptors & pipelines
    vkCmdBindIndexBuffer(cmd, (geometryPtr->indexBuffer)._buffer, 0, VK_INDEX_TYPE_UINT32);
    VkDeviceSize size = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &(geometryPtr->vertexBuffer)._buffer, &size);
    //vkCmdBindDescriptorSets()
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, (materialPtr->pipeline)._pipeline);

    // set custom props
    //vkCmdSetViewport();
    //vkCmdSetLineWidth();
    //vkCmdSetScissor();
    //vkCmdSetPrimitiveTopology();
    // draw
    vkCmdDrawIndexed(cmd, geometryPtr->indicesSize, 1, 0, 0, 0);
}

void RenderContext2D::recontruct()
{
    // TODO: build framebuffers
}

void RenderContext2D::Release()
{
    // TODO: release resources
}