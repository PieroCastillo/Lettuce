//
// Created by piero on 14/02/2024.
//
module;
#include <iostream>
#include "volk.h"

export module Lettuce:ComputePipeline;

import :Device;
import :Utils;
import :PipelineConnector;
import :Shader;
import :Swapchain;

export namespace Lettuce::Core
{
    class ComputePipeline
    {
    public:
        Device _device;
        VkPipelineLayout _pipelineLayout;
        VkPipeline _pipeline;

        void Build(Device &device, PipelineConnector &connector, Swapchain &swapchain, Shader &shader)
        {
            _pipelineLayout = connector._pipelineLayout;

            VkPipelineShaderStageCreateInfo pipelineShaderStageCI = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = (VkShaderStageFlagBits)shader._stage,
                .module = shader._shaderModule,
                .pName = shader._name.c_str(),
            };

            VkComputePipelineCreateInfo computePipelineCI = {
                .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
                // const void*                        pNext;
                // VkPipelineCreateFlags              flags;
                .stage = pipelineShaderStageCI,
                .layout = _pipelineLayout,
                // VkPipeline                         basePipelineHandle;
                // int32_t                            basePipelineIndex;
            };

            checkResult(vkCreateComputePipelines(_device._device, VK_NULL_HANDLE, 1, &computePipelineCI, nullptr, &_pipeline), "graphics pipeline created sucessfully");
        }

        void Destroy()
        {
            vkDestroyPipeline(_device._device, _pipeline, nullptr);
        }
    };
}