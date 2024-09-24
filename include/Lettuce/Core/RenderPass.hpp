//
// Created by piero on 1/09/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <tuple>
#include "Device.hpp"
#include "TextureView.hpp"

namespace Lettuce::Core
{
    class RenderPass
    {
    private:
        std::map<uint32_t, std::tuple<AttachmentType, VkAttachmentDescription, VkImageLayout>> attachments;
        std::vector<VkSubpassDescription> subpasses;
        std::map<uint32_t, std::tuple<BindPoint, std::vector<uint32_t>>> subpassesMap;
        std::vector<VkSubpassDependency> dependencies;
        std::vector<VkFramebufferCreateInfo> framebuffersCI;
        std::vector<std::vector<VkAttachmentReference>> attachmentsReferences; //to keep references alive
        std::vector<std::vector<VkImageView>> fbviews; //to keep the vector<imageView> alive
        void buildSubpasses();

    public:
        Device _device;
        VkRenderPass _renderPass;
        std::vector<VkFramebuffer> _framebuffers;

        void Build(Device& device);
        void Destroy();
        void BuildFramebuffers();
        void DestroyFramebuffers();
        void AddAttachment(
            uint32_t index,
            AttachmentType type,
            VkFormat format,
            LoadOp loadOp = LoadOp::Clear,
            StoreOp storeOp = StoreOp::Store,
            LoadOp stencilLoadOp = LoadOp::DontCare,
            StoreOp stencilStoreOp = StoreOp::DontCare,
            ImageLayout initial = ImageLayout::ColorAttachmentOptimal,
            ImageLayout final = ImageLayout::ColorAttachmentOptimal,
            ImageLayout reference = ImageLayout::ColorAttachmentOptimal);
        void AddSubpass(uint32_t index, BindPoint bindpoint, std::vector<uint32_t> attachments);
        void AddDependency(uint32_t srcSubpassIndex,
                           uint32_t dstSubpassIndex,
                           AccessStage srcStage,
                           AccessStage dstStage,
                           AccessBehavior srcBehavior,
                           AccessBehavior dstBehavior);
        void AddFramebuffer(uint32_t width, uint32_t height, std::vector<TextureView> attachments);
    };
}