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
        VkCommandBuffer _commandBuffer;
        uint32_t _count;

        Device _device;
        SynchronizationStructure _sync;

        void Create(Device &device, SynchronizationStructure &sync)
        {
            _sync = sync;
            _device = device;
        }

        void Begin()
        {
            VkCommandBufferBeginInfo cmdBeginCI = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, 
                .flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
            };
            checkResult(vkBeginCommandBuffer(_commandBuffer, &cmdBeginCI));
        }

        void End()
        {
            checkResult(vkEndCommandBuffer(_commandBuffer));
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

        void EndRendering(Swapchain swapchain)
        {
            vkCmdEndRendering(_commandBuffer);
        }

        void Send(int acquireImageSemaphoreIndex, int renderSemaphoreIndex, int fenceIndex)
        {
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

            checkResult(vkQueueSubmit(_device._graphicsQueue, 1, &submitI, _sync.fences[fenceIndex]), "cmd submitted successfully");
        }

        void BindGraphicsPipeline(GraphicsPipeline &pipeline)
        {
            vkCmdBindPipeline(_commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline._pipeline);
        }

        void BindDescriptorSetToGraphics(PipelineConnector &connector, Descriptor &descriptor)
        {
            vkCmdBindDescriptorSets(_commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, connector._pipelineLayout, 0, 1, &descriptor._descriptorSet, 0, nullptr);
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
            vkCmdBindVertexBuffers(_commandBuffer, 0, 1, &vertexBuffer._buffer, &offset);
        }

        void BindIndexBuffer(Buffer &indexBuffer, IndexType indexType)
        {
            vkCmdBindIndexBuffer(_commandBuffer, indexBuffer._buffer, 0, (VkIndexType)indexType);
        }

        void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
        {
            vkCmdDraw(_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
        }

        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0)
        {
            vkCmdDrawIndexed(_commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        }

        void SetScissor(Swapchain swapchain, int32_t xOffset = 0, int32_t yOffset = 0)
        {
            VkRect2D scissor = {
                .offset = {xOffset, yOffset},
                .extent = swapchain.extent,
            };
            vkCmdSetScissor(_commandBuffer, 0, 1, &scissor);
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

            vkCmdSetViewport(_commandBuffer, 0, 1, &viewport);
        }

        void SetLineWidth(float lineWidth)
        {
            vkCmdSetLineWidth(_commandBuffer, lineWidth);
        }

        void SetTopology(Topology topology)
        {
            vkCmdSetPrimitiveTopology(_commandBuffer, (VkPrimitiveTopology)topology);
        }

        template <typename T>
        void PushConstant(PipelineConnector &connector, PipelineStage stage, T &constant, uint32_t offset = 0)
        {
            vkCmdPushConstants(_commandBuffer, connector._pipelineLayout, (VkShaderStageFlags)stage, offset, sizeof(T), &constant);
        }

        void Reset()
        {
            vkResetCommandBuffer(_commandBuffer, 0);
        }
    };
}