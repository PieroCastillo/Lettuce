//
// Created by piero on 23/06/2024.
//
#include "Lettuce/Core/Common.hpp"
#include <iostream>
#include <vector>
#include <optional>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Descriptors.hpp"
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/PipelineLayout.hpp"

using namespace Lettuce::Core;

PipelineLayout::PipelineLayout(const std::shared_ptr<Device> &device, const std::shared_ptr<Descriptors> &descriptor)
    : _device(device), _descriptors(descriptor)
{
  
}

PipelineLayout::PipelineLayout(const std::shared_ptr<Device> &device) : _device(device)
{
   
}

void PipelineLayout::Release()
{
    vkDestroyPipelineLayout(_device->_device, _pipelineLayout, nullptr);
}

void PipelineLayout::AddPushConstant(uint32_t size, VkShaderStageFlags stages)
{
    VkPushConstantRange pushConstantRange = {
        .stageFlags = stages,
        .offset = currentOffset,
        .size = size,
    };
    currentOffset += size;
    pushConstants.emplace_back(pushConstantRange);
}

void PipelineLayout::Assemble()
{
     VkPipelineLayoutCreateInfo pipelineLayoutCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        //.flags = VkPipelineLayoutCreateFlagBits::VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT,
    };

    if (pushConstants.size() > 0)
    {
        pipelineLayoutCI.pPushConstantRanges = pushConstants.data();
        pipelineLayoutCI.pushConstantRangeCount = (uint32_t)pushConstants.size();
    }

    if (_descriptors && _descriptors->_pool != VK_NULL_HANDLE && _descriptors->_layouts.size() > 0)
    {
        pipelineLayoutCI.pSetLayouts = _descriptors->_layouts.data();
        pipelineLayoutCI.setLayoutCount = (uint32_t)_descriptors->_layouts.size();
    }

    checkResult(vkCreatePipelineLayout(_device->_device, &pipelineLayoutCI, nullptr, &_pipelineLayout));
}