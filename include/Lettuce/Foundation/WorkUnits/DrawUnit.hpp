//
// Created by piero on 28/02/2025.
//
#pragma once
#include <memory>
#include "Lettuce/Core/GraphicsPipeline.hpp"
#include "Lettuce/Core/PipelineLayout.hpp"
#include "Lettuce/Core/Descriptors.hpp"
#include "Lettuce/Foundation/IWorkUnit.hpp"

namespace Lettuce::Foundation
{
    struct DrawUnit : public IWorkUnit
    {
        std::shared_ptr<GraphicsPipeline> pipeline;
        std::shared_ptr<PipelineLayout> layout;
        std::shared_ptr<Descriptors> descriptors;
        int width, height;
        VkPrimitiveTopology topology;

        void Record(VkCommandBuffer cmd)
        {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->_pipeline);
            // VkDeviceSize _offset = offset;
            // VkDeviceSize size = 2 * sizeof(LineVertex);
            // vkCmdBindVertexBuffers2(cmd, 0, 1, &(deviceBuffer->_buffer), &_offset, &size, nullptr);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout->_pipelineLayout, 0, descriptors->_descriptorSets.size(), descriptors->_descriptorSets.data(), 0, nullptr);
            // dataPush.color = color;
            // vkCmdPushConstants(cmd, linesLayout->_pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DataPush), &dataPush);
            // vkCmdSetLineWidth(cmd, 5.0f);
            VkViewport viewport = {0, 0, (float)width, (float)height, 0.0f, 1.0f};
            vkCmdSetViewportWithCount(cmd, 1, &viewport);
            VkRect2D scissor = {{0, 0}, {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}};
            vkCmdSetScissorWithCount(cmd, 1, &scissor);
            vkCmdSetPrimitiveTopology(cmd, topology);
            vkCmdDraw(cmd, 2, 1, 0, 0);
        }
    };
};