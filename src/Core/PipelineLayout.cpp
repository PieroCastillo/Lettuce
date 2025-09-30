// standard headers
#include <vector>
#include <memory>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/PipelineLayout.hpp"

using namespace Lettuce::Core;

void PipelineLayout::Create(const std::weak_ptr<IDevice>& device, const PipelineLayoutCreateInfo& createInfo)
{
    m_device = (device.lock())->m_device;
    
    // TODO: Impl Buffer Address Table
    VkPipelineLayoutCreateInfo pipelineLayoutCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        // .setLayoutCount = (uint32_t)createInfo.layouts.size(),
        // .pSetLayouts = layouts.data(),
        // .pPushConstantRanges =  
    };

    handleResult(vkCreatePipelineLayout(m_device, &pipelineLayoutCI, nullptr, &m_pipelineLayout));
}

void PipelineLayout::Release()
{
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
}