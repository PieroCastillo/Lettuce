//
// Created by piero on 1/09/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <tuple>
#include "Device.hpp"

namespace Lettuce::Core
{
    /**
     * @brief RenderPass is an abstraction of VkRenderPass and VkFramebuffer[], also
     * automatizate the generation of AttachmentRefences.
     * Lifecycle:
     * renderPass.AddAttachments()
     * renderPass.AddSubpass()
     * renderPass.AddDependency()
     * no matter the order of submission before Assemble()
     * renderPass.Assemble() // here build the internal objects
     * renderPass.AddFramebuffer()
     * renderPass.BuildFramebuffers()
     * here finalize the setup
     * renderPass.DestroyFrambuffers()
     * renderPass.Destroy() //destroy and clear internal objects
     */
    class RenderPass
    {
    private:
        std::map<uint32_t, std::tuple<AttachmentType, VkAttachmentDescription, VkImageLayout>> attachments;
        std::vector<VkSubpassDescription> subpasses;
        std::vector<VkSubpassDependency> dependencies;
        std::map<uint32_t, std::tuple<BindPoint, std::vector<uint32_t>>> subpassesMap;
        std::vector<VkFramebufferCreateInfo> framebuffersCI;
        std::vector<std::vector<VkAttachmentReference>> attachmentReferencesStorage; // to keep references alive
        std::vector<std::vector<VkImageView>> fbviews;                               // to keep the vector<imageView> alive
        void buildSubpasses();

    public:
        std::shared_ptr<Device> _device;
        VkRenderPass _renderPass = VK_NULL_HANDLE;
        std::vector<VkFramebuffer> _framebuffers;

        RenderPass(const std::shared_ptr<Device> &device);
        ~RenderPass();
        void Assemble();
        void AddAttachment(uint32_t index,
                           AttachmentType type,
                           VkFormat format,
                           LoadOp loadOp,
                           StoreOp storeOp,
                           LoadOp stencilLoadOp,
                           StoreOp stencilStoreOp,
                           ImageLayout initial,
                           ImageLayout final,
                           ImageLayout reference);
        void AddSubpass(uint32_t index,
                        BindPoint bindpoint,
                        std::vector<uint32_t> attachments);

        void AddDependency(uint32_t srcSubpassIndex,
                           uint32_t dstSubpassIndex,
                           AccessStage srcStage,
                           AccessStage dstStage,
                           AccessBehavior srcBehavior,
                           AccessBehavior dstBehavior);
        void BuildFramebuffers();
        void DestroyFramebuffers();
        void AddFramebuffer(uint32_t width, uint32_t height, std::vector<VkImageView> attachments);
    };
}