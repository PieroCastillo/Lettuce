//
// Created by piero on 23/06/2024.
//
module;
#include <iostream>
#include <vector>
#define VOLK_IMPLEMENTATION
#include <volk.h>

export module Lettuce:PipelineConnector;

import :Device;
import :Utils;

export namespace Lettuce::Core
{
    
    typedef enum LettucePipelineStage
    {
        Vertex = 0x00000001,
        TessellationControl = 0x00000002,
        TessellationEvaluation = 0x00000004,
        Geometry = 0x00000008,
        Fragment = 0x00000010,
        Compute = 0x00000020,
        AllGraphics = 0x0000001F,
        All = 0x7FFFFFFF,
        Task = 0x00000040,
        Mesh = 0x00000080,
    };

    class PipelineConnector
    {
    public:
        Device _device;
        VkPipelineLayout _pipelineLayout;
        std::vector<VkPushConstantRange> pushConstants;
        //TODO: enable descriptor sets
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;

        template <typename T1>
        void AddPushConstant(uint32_t offset, LettucePipelineStage stage)
        {
            pushConstants.emplace_back({.stageFlags = stage,
                                        .offset = offset,
                                        .size = sizeof(T1)});
        }

        void Build(Device &device)
        {
            _device = device;

            VkPipelineLayoutCreateInfo pipelineLayoutCI = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            };

            if (pushConstants.size() > 0)
            {
                pipelineLayoutCI.pPushConstantRanges = pushConstants.data();
                pipelineLayoutCI.pushConstantRangeCount = (uint32_t)pushConstants.size();
            }

            if (descriptorSetLayouts.size() > 0)
            {
                pipelineLayoutCI.pSetLayouts = descriptorSetLayouts.data();
                pipelineLayoutCI.pushConstantRangeCount = (uint32_t)descriptorSetLayouts.size();
            }

            checkResult(vkCreatePipelineLayout(_device._device, &pipelineLayoutCI, nullptr, &_pipelineLayout), "PipelineLayout created sucessfully");
        }

        void Destroy()
        {
            vkDestroyPipelineLayout(_device._device, _pipelineLayout, nullptr);
        }
    };
}