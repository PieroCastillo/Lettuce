//
// Created by piero on 23/06/2024.
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <vector>
#include <optional>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Descriptors.hpp"
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/PipelineLayout.hpp"

using namespace Lettuce::Core;

void PipelineLayout::Build(const std::shared_ptr<Device> &device, Descriptors &descriptor)
{
    _device = device;

    VkPipelineLayoutCreateInfo pipelineLayoutCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        //.flags = VkPipelineLayoutCreateFlagBits::VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT,
    };

    if (pushConstants.size() > 0)
    {
        pipelineLayoutCI.pPushConstantRanges = pushConstants.data();
        pipelineLayoutCI.pushConstantRangeCount = (uint32_t)pushConstants.size();
    }

    if (descriptor._pool != VK_NULL_HANDLE && descriptor._layouts.size() > 0)
    {
        pipelineLayoutCI.pSetLayouts = descriptor._layouts.data();
        pipelineLayoutCI.setLayoutCount = (uint32_t)descriptor._layouts.size();
    }

    checkResult(vkCreatePipelineLayout(_device->_device, &pipelineLayoutCI, nullptr, &_pipelineLayout));
}


void PipelineLayout::Build(const std::shared_ptr<Device> &device)
{
    _device = device;

    VkPipelineLayoutCreateInfo pipelineLayoutCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        //.flags = VkPipelineLayoutCreateFlagBits::VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT,
    };

    if (pushConstants.size() > 0)
    {
        pipelineLayoutCI.pPushConstantRanges = pushConstants.data();
        pipelineLayoutCI.pushConstantRangeCount = (uint32_t)pushConstants.size();
    }
    
    checkResult(vkCreatePipelineLayout(_device->_device, &pipelineLayoutCI, nullptr, &_pipelineLayout));
}

void PipelineLayout::Destroy()
{
    vkDestroyPipelineLayout(_device->_device, _pipelineLayout, nullptr);
}