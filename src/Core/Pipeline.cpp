// standard headers

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/Pipeline.hpp"

Pipeline::Pipeline(VkDevice device, const GraphicsPipelineCreateInfo& createInfo, LettuceResult& result)   
{
    VkGraphicsPipelineCreateInfo gpipelineCI = {

    };
    
}

Pipeline::Pipeline(VkDevice device, const ComputePipelineCreateInfo& createInfo, LettuceResult& result)
{

}

void Pipeline::Release()
{
    
}