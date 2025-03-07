//
// Created by piero on 14/02/2024.
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <memory>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/PipelineLayout.hpp"
#include "Lettuce/Core/ShaderModule.hpp"
#include "Lettuce/Core/ComputePipeline.hpp"

using namespace Lettuce::Core;

ComputePipeline::ComputePipeline(const std::shared_ptr<Device> &device, const std::shared_ptr<PipelineLayout> &connector, const std::shared_ptr<ShaderModule> &shader)
{
    _device = device;
    _pipelineLayout = connector->_pipelineLayout;

    VkPipelineShaderStageCreateInfo pipelineShaderStageCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = (VkShaderStageFlagBits)shader->_stage,
        .module = shader->_shaderModule,
        .pName = shader->_name.c_str(),
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

ComputePipeline::~ComputePipeline()
{
    vkDestroyPipeline(_device->_device, _pipeline, nullptr);
}