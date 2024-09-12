//
// Created by piero on 23/06/2024.
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <vector>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/DescriptorLayout.hpp"
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/PipelineConnector.hpp"

using namespace Lettuce::Core;

void PipelineConnector::AddDescriptor(DescriptorLayout &descriptor)
{
    descriptorSetLayouts.emplace_back(descriptor._setLayout);
}

void PipelineConnector::Build(Device &device)
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

    if (descriptorSetLayouts.size() > 0)
    {
        pipelineLayoutCI.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutCI.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
    }

    checkResult(vkCreatePipelineLayout(_device._device, &pipelineLayoutCI, nullptr, &_pipelineLayout), "PipelineLayout created sucessfully");
}

void PipelineConnector::Destroy()
{
    vkDestroyPipelineLayout(_device._device, _pipelineLayout, nullptr);
}