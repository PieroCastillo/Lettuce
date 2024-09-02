//
// Created by piero on 1/09/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <tuple>
#include "Device.hpp"

namespace Lettuce::Core
{
    class RenderPass
    {
    private:
        std::map<int, std::vector<std::tuple<AttachmentType, VkAttachmentDescription>>> attachments;
        std::map<int, VkPipelineBindPoint> bindPoints;
        std::vector<VkSubpassDescription> subpasses;
        std::vector<VkSubpassDependency> dependencies;
        void buildSubpasses();

    public:
        Device _device;
        VkRenderPass _renderPass;
        VkFramebuffer _fb;

        void Build();
        void AddAttachment(
            int subpassIndex,
            AttachmentType type,
            VkFormat format,
            LoadOp loadOp = LoadOp::Clear,
            StoreOp storeOp = StoreOp::Store,
            LoadOp stencilLoadOp = LoadOp::DontCare,
            StoreOp stencilStoreOp = StoreOp::DontCare,
            ImageLayout initial = ImageLayout::ColorAttachmentOptimal,
            ImageLayout final = ImageLayout::ColorAttachmentOptimal);
        void BindSubpassIndex(int index, BindPoint point);
        void AddDependency(int srcSubpassIndex,
                           int dstSubpassIndex,
                           AccessStage srcStage,
                           AccessStage dstStage,
                           AccessBehavior srcBehavior,
                           AccessBehavior dstBehavior);
    };
}