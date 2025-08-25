/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_PIPELINE_LAYOUT_HPP
#define LETTUCE_CORE_PIPELINE_LAYOUT_HPP 

// standard headers
#include <vector>

// project headers
#include "Common.hpp"

namespace Lettuce::Core
{   
    struct PushConstant
    {
        uint32_t size;
        VkShaderStageFlags stages;
    };

    struct PipelineLayoutCreateInfo
    {
        std::vector<PushConstant> pushConstants;
        std::weak_ptr<DescriptorTable> descriptorTable;
    };

    class PipelineLayout
    {
    private:

    public:
        VkDevice m_device;
        VkPipelineLayout m_pipelineLayout;
            
        LettuceResult Create(const std::weak_ptr<IDevice>& device, const PipelineLayoutCreateInfo& createInfo);
        void Release();
    };
}
#endif // LETTUCE_CORE_PIPELINE_LAYOUT_HPP