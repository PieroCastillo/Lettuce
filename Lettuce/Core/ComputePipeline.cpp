//
// Created by piero on 14/02/2024.
//xmak
#include <iostream>
#include <memory>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/PipelineLayout.hpp"
#include "Lettuce/Core/ShaderModule.hpp"
#include "Lettuce/Core/ComputePipeline.hpp"

using namespace Lettuce::Core;

ComputePipeline::ComputePipeline(const std::shared_ptr<Device> &device, const std::shared_ptr<PipelineLayout> &layout, const std::shared_ptr<ShaderModule> &shader)
{
    _device = device;
    _pipelineLayout = layout;

    VkPipelineShaderStageCreateInfo pipelineShaderStageCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = (VkShaderStageFlagBits)shader->_stage,
        .module = shader->GetHandle(),
        .pName = shader->_name.c_str(),
    };

    VkComputePipelineCreateInfo computePipelineCI = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .stage = pipelineShaderStageCI,
        .layout = layout->GetHandle(),
        // VkPipeline                         basePipelineHandle;
        // int32_t                            basePipelineIndex;
    };

    checkResult(vkCreateComputePipelines(_device->GetHandle(), VK_NULL_HANDLE, 1, &computePipelineCI, nullptr, GetHandlePtr()));
}

void ComputePipeline::Release()
{
    vkDestroyPipeline(_device->GetHandle(), GetHandle(), nullptr);
}