/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_PIPELINE_HPP
#define LETTUCE_CORE_PIPELINE_HPP

// standard headers
#include <string>
#include <vector>

// project headers
#include "common.hpp"

namespace Lettuce::Core
{
    struct GraphicsPipelineCreateInfo
    {
        VkPrimitiveTopology topology;
        uint32_t patchControlPoints;
        bool fragmentShadingRate;
        std::vector<std::string> entryPoints;
        std::vector<VkShaderStageFlagBits> stages;
        std::vector<VkShaderModule> shaderModules;
        std::vector<VkFormat> colorAttachmentFormats;
        VkFormat depthAttachmentFormat;
        VkFormat stencilAttachmentFormat;
        VkPipelineLayout layout;
    };

    struct ComputePipelineCreateInfo
    {
        std::string entryPoint;
        VkShaderModule shaderModule;
        VkPipelineLayout layout;
    };

    class Pipeline
    {
    private:

    public:
        VkDevice m_device;
        VkPipeline m_pipeline;
        VkPipelineLayout m_layout;

        void Create(const IDevice& device, const GraphicsPipelineCreateInfo& createInfo);
        void Create(const IDevice& device, const ComputePipelineCreateInfo& createInfo);
        void Release();
    };
}
#endif // LETTUCE_CORE_PIPELINE_HPP