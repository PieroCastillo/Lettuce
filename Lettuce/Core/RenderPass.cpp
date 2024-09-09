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
    int subpassIndex,
    AttachmentType type,
    VkFormat format,
    LoadOp loadOp,
    StoreOp storeOp,
    LoadOp stencilLoadOp,
    StoreOp stencilStoreOp,
    ImageLayout initial,
    ImageLayout final)
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
    attachments[subpassIndex].push_back(std::make_tuple(type, attachment));
}

void RenderPass::BindSubpassIndex(int index, BindPoint point)
{
    bindPoints[index] = (VkPipelineBindPoint)point;
}

void RenderPass::buildSubpasses()
{
    int j = 0;
    for (auto element : attachments)
    {
        std::vector<VkAttachmentReference> colorRefs;
        std::vector<VkAttachmentReference> depthStencilRefs;
        std::vector<VkAttachmentReference> inputRefs;
        std::vector<VkAttachmentReference> resolveRefs;
        // std::vector<VkAttachmentReference> preserveRefs;
        for (int i = 0; i < element.second.size(); i++)
        {
            auto t = element.second[i];
            VkAttachmentDescription attachment;
            AttachmentType type;
            std::tie(type, attachment) = t;
            VkAttachmentReference ref = {
                .attachment = (uint32_t)i,
            };
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
            // case AttachmentType::Preserve:
            //     preserveRefs.push_back(ref);
            //     break;
            case AttachmentType::Resolve:
                resolveRefs.push_back(ref);
                break;
            }
        };

        VkSubpassDescription subpass;

        subpass.pipelineBindPoint = bindPoints[j];

        if (colorRefs.size() > 0)
        {
            subpass.colorAttachmentCount = (uint32_t)colorRefs.size();
            subpass.pColorAttachments = colorRefs.data();
        }

        if (depthStencilRefs.size() > 0)
        {
            subpass.pDepthStencilAttachment = depthStencilRefs.data();
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
        }

        if (resolveRefs.size() > 0)
        {
            subpass.pResolveAttachments = resolveRefs.data();
        }

        subpasses.push_back(subpass);
        j++;
    }
}

void RenderPass::AddDependency(int srcSubpassIndex,
                               int dstSubpassIndex,
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

void RenderPass::Build()
{
    VkRenderPassCreateInfo renderPassCI = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    };
    std::vector<VkAttachmentDescription> attachmentsVec(attachments.size());

    for (auto pair : attachments)
    {
        for (auto tuple : pair.second)
        {
            attachmentsVec.push_back(std::get<1>(tuple));
        }
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