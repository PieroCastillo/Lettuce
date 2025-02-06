//
// Created by piero on 14/02/2024.
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include "Lettuce/Core/ComputePipeline.hpp"

using namespace Lettuce::Core;

ComputePipeline(const std::shared_ptr<Device> &device, PipelineLayout &connector, Shader &shader)
{
    _device = device;
    _pipelineLayout = connector._pipelineLayout;

    VkPipelineShaderStageCreateInfo pipelineShaderStageCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = (VkShaderStageFlagBits)shader._stage,
        .module = shader._shaderModule,
        .pName = shader._name.c_str(),
    };

    VkComputePipelineCreateInfo computePipelineCI = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .stage = pipelineShaderStageCI,
        .layout = _pipelineLayout,
        // VkPipeline                         basePipelineHandle;
        // int32_t                            basePipelineIndex;
    };

    checkResult(vkCreateComputePipelines(_device->_device, VK_NULL_HANDLE, 1, &computePipelineCI, nullptr, &_pipeline));
}

~ComputePipeline::Destroy()
{
    vkDestroyPipeline(_device->_device, _pipeline, nullptr);
}