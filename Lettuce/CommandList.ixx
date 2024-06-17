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

export namespace Lettuce::Core
{
    enum QueueType
    {
        Graphics,
        Present
    };

    class CommandList
    {
    public:
        VkCommandPool _commandPool = VK_NULL_HANDLE;
        VkCommandBuffer _commandBuffer = VK_NULL_HANDLE;

        Device _device;
        SynchronizationStructure _sync;

        void Create(Device &device, SynchronizationStructure &sync, QueueType queueType = Graphics)
        {
            _sync = sync;
            _device = device;
            VkCommandPoolCreateInfo cmdPoolCI = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT};
            if (queueType == Graphics)
            {
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

        void Begin()
        {
            VkCommandBufferBeginInfo cmdBeginCI = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
            auto res = vkBeginCommandBuffer(_commandBuffer, &cmdBeginCI);
            std::cout << res << std::endl;
        }

        void End()
        {
            auto res = vkEndCommandBuffer(_commandBuffer);
            std::cout << res << std::endl;
        }

        void BeginRendering(Swapchain swapchain, float r = 1, float g = 1, float b = 1, float a = 1)
        {
            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {{r, g, b, a}};
            clearValues[1].depthStencil = {1.0f, 0};

            const VkRenderingAttachmentInfo colorAttachmentI{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = swapchain.swapChainImageViews[swapchain.index],
                .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = {{r, g, b, a}}};

            auto renderArea = VkRect2D{
                .offset = {0, 0},
                .extent = {swapchain.width, swapchain.height}};

            const VkRenderingInfo renderI{
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                .renderArea = renderArea,
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &colorAttachmentI,
            };

            vkCmdBeginRendering(_commandBuffer, &renderI);
        }

        void EndRendering()
        {
            vkCmdEndRendering(_commandBuffer);
        }

        void Send(int acquireImageSemaphoreIndex, int renderSemaphoreIndex, int fenceIndex)
        {
            // VkSemaphoreSubmitInfo waitSemaphoreSubmitInfo = {
            //     .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            //     .semaphore = _sync.semaphores[acquireImageSemaphoreIndex],
            //     .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT};
            // VkSemaphoreSubmitInfo signalSemaphoreSubmitInfo = {
            //     .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            //     .semaphore = _sync.semaphores[renderSemaphoreIndex]};

            // VkCommandBufferSubmitInfo cmdSubmitInfo = {
            //     .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            //     .commandBuffer = _commandBuffer};

            VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
            VkSubmitInfo submitI = {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &(_sync.semaphores[acquireImageSemaphoreIndex]),
                .pWaitDstStageMask = waitStages,
                .commandBufferCount = 1,
                .pCommandBuffers = &_commandBuffer,
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = &(_sync.semaphores[renderSemaphoreIndex]),
            };

            // VkSubmitInfo2 graphicsSubmitInfo = {
            //     .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            //     .waitSemaphoreInfoCount = 1,
            //     .pWaitSemaphoreInfos = &waitSemaphoreSubmitInfo,
            //     .commandBufferInfoCount = 1,
            //     .pCommandBufferInfos = &cmdSubmitInfo,
            //     .signalSemaphoreInfoCount = 1,
            //     .pSignalSemaphoreInfos = &signalSemaphoreSubmitInfo,
            // };
            // auto res = vkQueueSubmit2(_device._graphicsQueue, 1, &graphicsSubmitInfo, VK_NULL_HANDLE);
            auto res = vkQueueSubmit(_device._graphicsQueue, 1, &submitI, _sync.fences[fenceIndex]);
            std::cout << res << std::endl;
        }

        void Reset()
        {
            vkResetCommandBuffer(_commandBuffer, 0);
        }

        void Destroy()
        {
            vkFreeCommandBuffers(_device._device, _commandPool, 1, &_commandBuffer);
            vkDestroyCommandPool(_device._device, _commandPool, nullptr);
        }
    };
}