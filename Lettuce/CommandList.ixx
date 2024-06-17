//
// Created by piero on 14/02/2024.
//
module;
#include <iostream>
#include <array>
#include <vulkan/vulkan.h>

export module Lettuce:CommandList;

import :Utils;
import :Device;
import :Swapchain;
import :GPU;
import :SynchronizationStructure;

using namespace std;

export namespace Lettuce::Core{
    enum QueueType {
        Graphics,
        Present
    };

    class CommandList{
    public:
        VkCommandPool _commandPool = VK_NULL_HANDLE;
        VkCommandBuffer _commandBuffer = VK_NULL_HANDLE;

        Device _device;
        SynchronizationStructure _sync;

        void Create(Device &device, SynchronizationStructure &sync, QueueType queueType = Graphics) {
            _sync = sync;
            _device = device;
            VkCommandPoolCreateInfo cmdPoolCI = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
            };
            if(queueType == Graphics) {
                cmdPoolCI.queueFamilyIndex = _device._gpu.graphicsFamily.value();
            }
            std::cout << "papu" << std::endl;
            checkResult(vkCreateCommandPool(_device._device, &cmdPoolCI, nullptr, &_commandPool), "CommandPool created successfully");

            VkCommandBufferAllocateInfo cmdBufferAI = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = _commandPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };
            checkResult(vkAllocateCommandBuffers(_device._device, &cmdBufferAI, &_commandBuffer), "CommandBuffer allocated successfully");
        }

        void Begin() {
            VkCommandBufferBeginInfo cmdBeginCI = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
            };
            auto res = vkBeginCommandBuffer(_commandBuffer, &cmdBeginCI);
            std::cout << res << std::endl;
        }

        void End() {
            auto res = vkEndCommandBuffer(_commandBuffer);
            std::cout << res << std::endl;
        }

        void BeginRendering(Swapchain swapchain, float r = 1, float g = 1, float b = 1, float a = 1) {
            std::array<VkClearValue,2> clearValues{};
            clearValues[0].color        = {{r, g, b, a}};
            clearValues[1].depthStencil = {1.0f, 0};

            // VkImageSubresourceRange range = { VK_IMAGE_ASPECT_COLOR_BIT,0,VK_REMAINING_MIP_LEVELS,0,VK_REMAINING_ARRAY_LAYERS};
            // VkImageSubresourceRange depth_range{range};
            // depth_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            //barrier

            VkRenderingAttachmentInfo depthAttachmentInfo ={
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .clearValue = clearValues[1],
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                .imageView = swapchain.dep

            };

            VkRenderingAttachmentInfoKHR color_attachment_info = {
                .sType                        = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView                    = swapchain.swapChainImageViews[swapchain.index],        // color_attachment.image_view;
                .imageLayout                  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode                  = VK_RESOLVE_MODE_NONE,
                .loadOp                       = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp                      = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue                   = clearValues[0]
            };
            //
            // VkRenderingAttachmentInfoKHR depth_attachment_info = {
            //     .sType                        = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            //     .imageView                    = depth_stencil.view,
            //     .imageLayout                  = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            //     .resolveMode                  = VK_RESOLVE_MODE_NONE,
            //     .loadOp                       = VK_ATTACHMENT_LOAD_OP_CLEAR,
            //     .storeOp                      = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            //     .clearValue                   = clearValues[1],
            // };

            auto render_area = VkRect2D{VkOffset2D{}, VkExtent2D{swapchain.width, swapchain.height}};
            VkRenderingInfoKHR renderI = {
                .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
                .renderArea           = render_area,
                .layerCount       = 1,
                .viewMask             = 0,
                .colorAttachmentCount = 1,
                .pColorAttachments    = &color_attachment_info,
                //.pStencilAttachment   = VK_NULL_HANDLE,
                //.pDepthAttachment = &depth_attachment_info,
            };
            // if (!(depth_format == VK_FORMAT_D16_UNORM || depth_format == VK_FORMAT_D32_SFLOAT))
            // {
            //     renderI.pStencilAttachment = &depth_attachment_info;
            // }

            vkCmdBeginRendering(_commandBuffer, &renderI);
        }

        void EndRendering(Swapchain swapchain) {
            vkCmdEndRendering(_commandBuffer);
        }

        void Send(int acquireImageSemaphoreIndex, int renderSemaphoreIndex) {
            VkSemaphoreSubmitInfo waitSemaphoreSubmitInfo = {
                .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = _sync.semaphores[acquireImageSemaphoreIndex],
                .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
            };
            VkSemaphoreSubmitInfo signalSemaphoreSubmitInfo = {
                .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = _sync.semaphores[renderSemaphoreIndex]
            };

            VkCommandBufferSubmitInfo cmdSubmitInfo = {
                .sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                .commandBuffer = _commandBuffer
            };

            VkSubmitInfo2 graphicsSubmitInfo = {
                .sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                .waitSemaphoreInfoCount   = 1,
                .pWaitSemaphoreInfos      = &waitSemaphoreSubmitInfo,
                .commandBufferInfoCount   = 1,
                .pCommandBufferInfos      = &cmdSubmitInfo,
                .signalSemaphoreInfoCount = 1,
                .pSignalSemaphoreInfos    = &signalSemaphoreSubmitInfo,
            };
            auto res = vkQueueSubmit2(_device._graphicsQueue, 1, &graphicsSubmitInfo, VK_NULL_HANDLE);
            std::cout << res << std::endl;
        }

        void Reset(){
            vkResetCommandBuffer(_commandBuffer, 0);
        }

        void Destroy(){
            vkFreeCommandBuffers(_device._device, _commandPool, 1, &_commandBuffer);
            vkDestroyCommandPool(_device._device, _commandPool, nullptr);
        }
    };
}