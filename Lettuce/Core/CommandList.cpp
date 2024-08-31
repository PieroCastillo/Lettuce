//
// Created by piero on 14/02/2024.
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <array>
#include <vector>
#include <algorithm>
#include <cstdint>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/Buffer.hpp"
#include "Lettuce/Core/Swapchain.hpp"
#include "Lettuce/Core/Semaphore.hpp"
#include "Lettuce/Core/CommandList.hpp"
#include "Lettuce/Core/Descriptor.hpp"
#include "Lettuce/Core/ComputePipeline.hpp"
#include "Lettuce/Core/GraphicsPipeline.hpp"
#include "Lettuce/Core/PipelineConnector.hpp"

using namespace Lettuce::Core;

void CommandList::Begin()
{
    VkCommandBufferBeginInfo cmdBeginCI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
    };
    checkResult(vkBeginCommandBuffer(_commandBuffer, &cmdBeginCI));
}

void CommandList::End()
{
    checkResult(vkEndCommandBuffer(_commandBuffer));
}

void CommandList::BindGraphicsPipeline(GraphicsPipeline &pipeline)
{
    vkCmdBindPipeline(_commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline._pipeline);
}

void CommandList::BindDescriptorSetToGraphics(PipelineConnector &connector, Descriptor &descriptor)
{
    vkCmdBindDescriptorSets(_commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, connector._pipelineLayout, 0, 1, &descriptor._descriptorSet, 0, nullptr);
}

void CommandList::BindDescriptorSetToCompute(PipelineConnector &connector, Descriptor &descriptor)
{
    vkCmdBindDescriptorSets(_commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, connector._pipelineLayout, 0, 1, &descriptor._descriptorSet, 0, nullptr);
}

void CommandList::BindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, std::vector<Buffer> &buffers, std::vector<uint32_t> &offsets)
{
    // std::vector<VkBuffer> _buffers;
    // for (auto buffer : buffers)
    // {
    //     _buffers.emplace_back(buffer);
    // }
    // vkCmdBindVertexBuffers(_commandBuffer[index], firstBinding, bindingCount, , offsets.data());
}

void CommandList::BindVertexBuffer(Buffer &vertexBuffer)
{
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(_commandBuffer, 0, 1, &vertexBuffer._buffer, &offset);
}

void CommandList::BindIndexBuffer(Buffer &indexBuffer, IndexType indexType)
{
    vkCmdBindIndexBuffer(_commandBuffer, indexBuffer._buffer, 0, (VkIndexType)indexType);
}

void CommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    vkCmdDraw(_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
    vkCmdDrawIndexed(_commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandList::DrawIndirect(Buffer indirectBuffer, uint32_t drawCount, uint32_t stride, uint32_t offset)
{
    vkCmdDrawIndirect(_commandBuffer, indirectBuffer._buffer, offset, drawCount, stride);
}

void CommandList::DrawMesh(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    vkCmdDrawMeshTasksEXT(_commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void CommandList::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    vkCmdDispatch(_commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void CommandList::SetScissor(Swapchain swapchain, int32_t xOffset, int32_t yOffset)
{
    VkRect2D scissor = {
        .offset = {xOffset, yOffset},
        .extent = swapchain.extent,
    };
    vkCmdSetScissor(_commandBuffer, 0, 1, &scissor);
}

void CommandList::SetViewport(float width, float height, float minDepth, float maxDepth, float x, float y)
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

void CommandList::SetLineWidth(float lineWidth)
{
    vkCmdSetLineWidth(_commandBuffer, lineWidth);
}

void CommandList::SetTopology(Topology topology)
{
    vkCmdSetPrimitiveTopology(_commandBuffer, (VkPrimitiveTopology)topology);
}

template <typename T>
void CommandList::PushConstant(PipelineConnector &connector, PipelineStage stage, T &constant, uint32_t offset)
{
    vkCmdPushConstants(_commandBuffer, connector._pipelineLayout, (VkShaderStageFlags)stage, offset, sizeof(T), &constant);
}

void CommandList::Reset()
{
    vkResetCommandBuffer(_commandBuffer, 0);
}