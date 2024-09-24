//
// Created by piero on 1/09/2024.
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <tuple>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/RenderPass.hpp"

using namespace Lettuce::Core;

void RenderPass::AddAttachment(
    uint32_t index,
    AttachmentType type,
    VkFormat format,
    LoadOp loadOp,
    StoreOp storeOp,
    LoadOp stencilLoadOp,
    StoreOp stencilStoreOp,
    ImageLayout initial,
    ImageLayout final,
    ImageLayout reference)
{
    VkAttachmentDescription attachment = {
        .format = format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = (VkAttachmentLoadOp)loadOp,
        .storeOp = (VkAttachmentStoreOp)storeOp,
        .stencilLoadOp = (VkAttachmentLoadOp)stencilLoadOp,
        .stencilStoreOp = (VkAttachmentStoreOp)stencilStoreOp,
        .initialLayout = (VkImageLayout)initial,
        .finalLayout = (VkImageLayout) final,
    };
    attachments[index] = std::make_tuple(type, attachment, (VkImageLayout)reference);
}

void RenderPass::buildSubpasses()
{
    for (auto [index, att] : subpassesMap)
    {
        std::vector<VkAttachmentReference> colorRefs;
        std::vector<VkAttachmentReference> depthStencilRefs;
        std::vector<VkAttachmentReference> inputRefs;
        std::vector<VkAttachmentReference> resolveRefs;
        auto [bindPoint, attIndices] = att;
        // std::vector<VkAttachmentReference> preserveRefs;
        for (auto attIndex : attIndices)
        {
            auto [type, attachmentDesc, refLayout] = attachments[attIndex];
            VkAttachmentReference ref;
            ref.attachment = attIndex;
            ref.layout = refLayout;
            switch (type)
            {
            case AttachmentType::Color:
                colorRefs.push_back(ref);
                break;
            case AttachmentType::DepthStencil:
                depthStencilRefs.push_back(ref);
                break;
            case AttachmentType::Input:
                inputRefs.push_back(ref);
                break;
            case AttachmentType::Resolve:
                resolveRefs.push_back(ref);
                break;
            }
        };

        VkSubpassDescription subpass;
        // subpass.flags = 
        subpass.pipelineBindPoint = (VkPipelineBindPoint)bindPoint;

        if (colorRefs.size() > 0)
        {
            subpass.colorAttachmentCount = (uint32_t)colorRefs.size();
            subpass.pColorAttachments = colorRefs.data();
            attachmentsReferences.push_back(colorRefs);
        }

        if (depthStencilRefs.size() > 0)
        {
            subpass.pDepthStencilAttachment = depthStencilRefs.data();
            attachmentsReferences.push_back(depthStencilRefs);
        }

        // if (preserveRefs.size() > 0)
        // {
        //     subpass.preserveAttachmentCount = (uint32_t)preserveRefs.size();
        //     subpass.pPreserveAttachments = preserveRefs.data();
        // }

        if (inputRefs.size() > 0)
        {
            subpass.inputAttachmentCount = (uint32_t)inputRefs.size();
            subpass.pInputAttachments = inputRefs.data();
            attachmentsReferences.push_back(inputRefs);
        }

        if (resolveRefs.size() > 0)
        {
            subpass.pResolveAttachments = resolveRefs.data();
            attachmentsReferences.push_back(resolveRefs);
        }

        subpasses.push_back(subpass);
    }
}

void RenderPass::AddDependency(uint32_t srcSubpassIndex,
                               uint32_t dstSubpassIndex,
                               AccessStage srcStage,
                               AccessStage dstStage,
                               AccessBehavior srcBehavior,
                               AccessBehavior dstBehavior)
{
    VkSubpassDependency dependency = {
        .srcSubpass = (uint32_t)srcSubpassIndex,
        .dstSubpass = (uint32_t)dstSubpassIndex,
        .srcStageMask = (VkPipelineStageFlags)srcStage,
        .dstStageMask = (VkPipelineStageFlags)dstStage,
        .srcAccessMask = (VkAccessFlags)srcBehavior,
        .dstAccessMask = (VkAccessFlags)dstBehavior,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
    };
    dependencies.push_back(dependency);
}

void RenderPass::AddSubpass(uint32_t index, BindPoint bindpoint, std::vector<uint32_t> attachments)
{
    subpassesMap[index] = std::make_tuple(bindpoint, attachments);
}

void RenderPass::AddFramebuffer(uint32_t width, uint32_t height, std::vector<TextureView> attachments)
{
    std::vector<VkImageView> views;
    for (auto view : attachments)
    {
        views.push_back(view._imageView);
    }
    fbviews.push_back(views);
    VkFramebufferCreateInfo framebufferCI = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = _renderPass,
        .attachmentCount = (uint32_t)views.size(),
        .pAttachments = views.data(),
        .width = width,
        .height = height,
        .layers = 1,
    };
    framebuffersCI.push_back(framebufferCI);
}

void RenderPass::Build(Device& device)
{
    _device = device;
    VkRenderPassCreateInfo renderPassCI = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    };
    buildSubpasses();
    std::vector<VkAttachmentDescription> attachmentsVec;

    for (auto [index, att] : attachments)
    {
        auto [type, desc, refLayout] = att;
        attachmentsVec.push_back(desc);
    }

    if (attachments.size() > 0)
    {
        renderPassCI.attachmentCount = (uint32_t)attachmentsVec.size();
        renderPassCI.pAttachments = attachmentsVec.data();
    }
    if (subpasses.size() > 0)
    {
        renderPassCI.subpassCount = (uint32_t)subpasses.size();
        renderPassCI.pSubpasses = subpasses.data();
    }
    if (dependencies.size() > 0)
    {
        renderPassCI.dependencyCount = (uint32_t)dependencies.size();
        renderPassCI.pDependencies = dependencies.data();
    }

    checkResult(vkCreateRenderPass(_device._device, &renderPassCI, nullptr, &_renderPass));
}

void RenderPass::Destroy()
{
    vkDestroyRenderPass(_device._device, _renderPass, nullptr);
    attachments.clear();
    attachmentsReferences.clear();
    subpasses.clear();
    subpassesMap.clear();
    dependencies.clear();
    framebuffersCI.clear();
}

void RenderPass::BuildFramebuffers()
{
    _framebuffers.clear();
    _framebuffers.resize(framebuffersCI.size());
    int i = 0;
    for (auto framebufferCI : framebuffersCI)
    {
        VkFramebuffer fb;
        checkResult(vkCreateFramebuffer(_device._device, &framebufferCI, nullptr, &fb));
        _framebuffers[i] = fb;
        i++;
    }
    framebuffersCI.clear();
}
void RenderPass::DestroyFramebuffers()
{
    for (auto fb : _framebuffers)
    {
        vkDestroyFramebuffer(_device._device, fb, nullptr);
    }
    _framebuffers.clear();
    fbviews.clear();
}