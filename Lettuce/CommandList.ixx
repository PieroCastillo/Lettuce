//
// Created by piero on 14/02/2024.
//
module;
#include <iostream>
#include <array>
#define VOLK_IMPLEMENTATION
#include <volk.h>
#include <algorithm>
#include <cstdint>

export module Lettuce:CommandList;

import :Utils;
import :Device;
import :Swapchain;
import :GPU;
import :SynchronizationStructure;
import :GraphicsPipeline;
import :Buffer;
import :Descriptor;

export namespace Lettuce::Core
{
    enum class IndexType
    {
        UInt16 = 0,
        UInt32 = 1,
        UInt8 = 1000265000, // Provided by VK_KHR_index_type_uint8
    };

    enum class QueueType
    {
        Graphics,
        Present
    };

    enum class Topology
    {
        PointList = 0,
        LineList = 1,
        LineStrip = 2,
        TriangleList = 3,
        TriangleStrip = 4,
        TriangleFan = 5,
        LineListWithAdjacency = 6,
        LineStripWithAdjacency = 7,
        TriangleListWithAdjacency = 8,
        TriangleStripWithAdjacency = 9,
        PatchList = 10,
    };

    class CommandList
    {
    public:
        VkCommandPool _commandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> _commandBuffer;
        uint32_t _count;
        int index = 0;

        Device _device;
        SynchronizationStructure _sync;

        void Create(Device &device, SynchronizationStructure &sync, QueueType queueType = QueueType::Graphics, uint32_t count = 1)
        {
            _count = std::max(count, 1U);
            _sync = sync;
            _device = device;
            VkCommandPoolCreateInfo cmdPoolCI = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            };
            if (queueType == QueueType::Graphics)
            {
                cmdPoolCI.queueFamilyIndex = _device._gpu.graphicsFamily.value();
            }
            checkResult(vkCreateCommandPool(_device._device, &cmdPoolCI, nullptr, &_commandPool), "CommandPool created successfully");

            VkCommandBufferAllocateInfo cmdBufferAI = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = _commandPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = _count,
            };
            _commandBuffer.resize(count);
            checkResult(vkAllocateCommandBuffers(_device._device, &cmdBufferAI, _commandBuffer.data()), "CommandBuffer allocated successfully");
        }

        void Begin()
        {
            VkCommandBufferBeginInfo cmdBeginCI = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
            vkBeginCommandBuffer(_commandBuffer[index], &cmdBeginCI);
        }

        void End()
        {
            vkEndCommandBuffer(_commandBuffer[index]);
        }

        void BeginRendering(Swapchain swapchain, float r = 1, float g = 1, float b = 1, float a = 1)
        {
            const VkImageMemoryBarrier image_memory_barrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .image = swapchain.swapChainImages[swapchain.index],
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                }};

            vkCmdPipelineBarrier(
                _commandBuffer[index],
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,             // srcStageMask
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // dstStageMask
                0,
                0,
                nullptr,
                0,
                nullptr,
                1,                    // imageMemoryBarrierCount
                &image_memory_barrier // pImageMemoryBarriers
            );

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

            vkCmdBeginRendering(_commandBuffer[index], &renderI);
        }

        void EndRendering(Swapchain swapchain)
        {
            vkCmdEndRendering(_commandBuffer[index]);

            const VkImageMemoryBarrier image_memory_barrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                .image = swapchain.swapChainImages[swapchain.index],
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                }};

            vkCmdPipelineBarrier(
                _commandBuffer[index],
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // srcStageMask
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,          // dstStageMask
                0,
                0,
                nullptr,
                0,
                nullptr,
                1,                    // imageMemoryBarrierCount
                &image_memory_barrier // pImageMemoryBarriers
            );
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
            //     .commandBuffer = _commandBuffer[index]};

            VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
            VkSubmitInfo submitI = {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &(_sync.semaphores[acquireImageSemaphoreIndex]),
                .pWaitDstStageMask = waitStages,
                .commandBufferCount = 1,
                .pCommandBuffers = &_commandBuffer[index],
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
            vkQueueSubmit(_device._graphicsQueue, 1, &submitI, _sync.fences[fenceIndex]);
            // std::cout << res << std::endl;
        }

        void BindGraphicsPipeline(GraphicsPipeline &pipeline)
        {
            vkCmdBindPipeline(_commandBuffer[index], VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline._pipeline);
        }

        void BindDescriptorSetToGraphics(PipelineConnector &connector, Descriptor &descriptor)
        {
            vkCmdBindDescriptorSets(_commandBuffer[index], VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, connector._pipelineLayout, 0, 1, &descriptor._descriptorSet, 0, nullptr);
        }

        void BindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, std::vector<Buffer> &buffers, std::vector<uint32_t> &offsets)
        {
            // std::vector<VkBuffer> _buffers;
            // for (auto buffer : buffers)
            // {
            //     _buffers.emplace_back(buffer);
            // }
            // vkCmdBindVertexBuffers(_commandBuffer[index], firstBinding, bindingCount, , offsets.data());
        }

        void BindVertexBuffer(Buffer &vertexBuffer)
        {
            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(_commandBuffer[index], 0, 1, &vertexBuffer._buffer, &offset);
        }

        void BindIndexBuffer(Buffer &indexBuffer, IndexType indexType)
        {
            vkCmdBindIndexBuffer(_commandBuffer[index], indexBuffer._buffer, 0, (VkIndexType)indexType);
        }

        void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
        {
            vkCmdDraw(_commandBuffer[index], vertexCount, instanceCount, firstVertex, firstInstance);
        }

        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0)
        {
            vkCmdDrawIndexed(_commandBuffer[index], indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        }

        void SetScissor(Swapchain swapchain, int32_t xOffset = 0, int32_t yOffset = 0)
        {
            VkRect2D scissor = {
                .offset = {xOffset, yOffset},
                .extent = swapchain.extent,
            };
            vkCmdSetScissor(_commandBuffer[index], 0, 1, &scissor);
        }

        void SetViewport(float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f, float x = 0.0f, float y = 0.0f)
        {
            VkViewport viewport = {
                .x = x,
                .y = y,
                .width = width,
                .height = height,
                .minDepth = minDepth,
                .maxDepth = maxDepth,
            };

            vkCmdSetViewport(_commandBuffer[index], 0, 1, &viewport);
        }

        void SetLineWidth(float lineWidth)
        {
            vkCmdSetLineWidth(_commandBuffer[index], lineWidth);
        }

        void SetTopology(Topology topology)
        {
            vkCmdSetPrimitiveTopology(_commandBuffer[index], (VkPrimitiveTopology)topology);
        }

        template <typename T>
        void PushConstant(PipelineConnector &connector, PipelineStage stage, T &constant, uint32_t offset = 0)
        {
            vkCmdPushConstants(_commandBuffer[index], connector._pipelineLayout, (VkShaderStageFlags)stage, offset, sizeof(T), &constant);
        }

        void Reset()
        {
            vkResetCommandBuffer(_commandBuffer[index], 0);
        }

        void Destroy()
        {
            for (uint32_t i = 0; i < _count; i++)
            {
                vkFreeCommandBuffers(_device._device, _commandPool, 1, &_commandBuffer[i]);
            }
            vkDestroyCommandPool(_device._device, _commandPool, nullptr);
        }
    };
}