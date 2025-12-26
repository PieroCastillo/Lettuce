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
    struct MeshShadingPipelineCreateInfo
    {
        bool fragmentShadingRate;
        std::string taskEntryPoint;
        std::string meshEntryPoint;
        std::string fragEntryPoint;
        VkShaderModule taskShaderModule;
        VkShaderModule meshShaderModule;
        VkShaderModule fragShaderModule;
        std::vector<VkFormat> colorAttachmentFormats;
        VkFormat depthAttachmentFormat;
        VkFormat stencilAttachmentFormat;
        VkPipelineLayout layout;
    };

    struct PrimitiveShadingPipelineCreateInfo
    {
        VkPrimitiveTopology topology;
        bool fragmentShadingRate;
        std::string vertEntryPoint;
        std::string fragEntryPoint;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;
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
        VkDevice m_device;
        VkPipeline m_pipeline;
        VkPipelineLayout m_layout;
    public:
        void Create(const IDevice& device, const MeshShadingPipelineCreateInfo& createInfo);
        void Create(const IDevice& device, const PrimitiveShadingPipelineCreateInfo& createInfo);
        void Create(const IDevice& device, const ComputePipelineCreateInfo& createInfo);
        void Release();

        inline uint64_t GetPipelineHandle() { return (uint64_t)m_pipeline; }
        inline uint64_t GetPipelineLayoutHandle() { return (uint64_t)m_layout; }
    };
}
#endif // LETTUCE_CORE_PIPELINE_HPP