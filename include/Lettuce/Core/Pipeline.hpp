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
        VkPrimitiveTopology topology;
        uint32_t patchControlPoints;
        bool fragmentShadingRate;
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

        void Create(const std::weak_ptr<IDevice>& device, const GraphicsPipelineCreateInfo& createInfo);
        void Create(const std::weak_ptr<IDevice>& device, const ComputePipelineCreateInfo& createInfo);
        void Release();
    };
}
#endif // LETTUCE_CORE_PIPELINE_HPP