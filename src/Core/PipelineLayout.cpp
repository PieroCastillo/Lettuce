// standard headers
#include <vector>
#include <memory>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/PipelineLayout.hpp"

using namespace Lettuce::Core;

LettuceResult PipelineLayout::Create(const std::weak_ptr<IDevice>& device, const PipelineLayoutCreateInfo& createInfo)
{
    m_device = device->m_device;
    
    // TODO: Impl Buffer Address Table
    VkPipelineLayoutCreateInfo pipelineLayoutCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = (uint32_t)layouts.size(),
        .pSetLayouts = layouts.data(),
        // .pPushConstantRanges =  
    };

    auto result = vkCreatePipelineLayout(m_device, &pipelineLayoutCI, nullptr, &m_pipelineLayout);

    return LettuceResult::Success;
}

void PipelineLayout::Release()
{
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
}