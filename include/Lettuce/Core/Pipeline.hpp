/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_PIPELINE_HPP
#define LETTUCE_CORE_PIPELINE_HPP 

// project headers
#include "Common.hpp"

namespace Lettuce::Core
{
    struct GraphicsPipelineCreateInfo
    {
        
    };

    struct ComputePipelineCreateInfo
    {
        
    };

    class Pipeline
    {
    private:
    
    public:
        VkDevice m_device;
        VkPipeline m_pipeline;

        Pipeline(VkDevice device, const GraphicsPipelineCreateInfo& createInfo);
        Pipeline(VkDevice device, const ComputePipelineCreateInfo& createInfo);
        void Release();
    };
}
#endif // LETTUCE_CORE_PIPELINE_HPP