//
// Created by piero on 14/10/2024.
//
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Lettuce/Core/common.hpp"
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
    // color attachment
    renderPass.AddAttachment(1, AttachmentType::Color,
                             swapchainImageFormat,
                             LoadOp::DontCare, StoreOp::Store,
                             LoadOp::None, StoreOp::None,
                             ImageLayout::ColorAttachmentOptimal, ImageLayout::ReadOnlyOptimal, ImageLayout::ColorAttachmentOptimal);

    // depth attachment
    renderPass.AddAttachment(2, AttachmentType::DepthStencil,
                             device._gpu.GetDepthFormat(),
                             LoadOp::DontCare, StoreOp::Store,
                             LoadOp::DontCare, StoreOp::Store,
                             ImageLayout::DepthStencilAttachmentOptimal, ImageLayout::DepthAttachmentStencilReadOnlyOptimal, ImageLayout::DepthStencilAttachmentOptimal);

    renderPass.AddSubpass(0, BindPoint::Graphics, {1, 2});
    renderPass.AddSubpass(1, BindPoint::Graphics, {0,1,2});

    renderPass.AddDependency(0, VK_SUBPASS_EXTERNAL, )
}
void RenderContext2D::Record(VkCommandBuffer cmd) {}
void RenderContext2D::recontruct() {}