//
// Created by piero on 14/02/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include <vector>
#include <memory>
#include "Device.hpp"
#include "Shader.hpp"
#include "RenderPass.hpp"
#include "Utils.hpp"
#include "PipelineLayout.hpp"

namespace Lettuce::Core
{
    // TODO: implement PipelineCache, ShaderStages, DynamicState
    class GraphicsPipeline
    {
    public:
        struct PipelineBuildData
        {
            struct InputAssemblyState
            {
                VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                VkBool32 primitiveRestartEnable;
            } inputAssembly;
            struct ViewportState
            {
                std::vector<VkViewport> viewports;
                std::vector<VkRect2D> scissors;
            } viewport;
            struct RasterizationState
            {
                VkBool32 depthClampEnable;
                VkBool32 rasterizerDiscardEnable;
                VkPolygonMode polygonMode;
                VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
                VkFrontFace frontFace = VkFrontFace::VK_FRONT_FACE_CLOCKWISE;
                VkBool32 depthBiasEnable;
                float depthBiasConstantFactor;
                float depthBiasClamp;
                float depthBiasSlopeFactor;
                float lineWidth;
            } rasterization;
            struct MultisampleState
            {
                VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
                VkBool32 sampleShadingEnable;
                float minSampleShading;
                VkSampleMask *pSampleMask;
                VkBool32 alphaToCoverageEnable;
                VkBool32 alphaToOneEnable;
            } multisample;
            struct DepthStencilState
            {
                VkBool32 depthTestEnable;
                VkBool32 depthWriteEnable;
                VkCompareOp depthCompareOp;
                VkBool32 depthBoundsTestEnable;
                VkBool32 stencilTestEnable;
                VkStencilOpState front;
                VkStencilOpState back;
                float minDepthBounds;
                float maxDepthBounds;
            } depthStencil;
            struct ColorBlendState
            {
                VkBool32 logicOpEnable;
                VkLogicOp logicOp;
                std::vector<VkPipelineColorBlendAttachmentState> attachments;
                float blendConstants[4];
            } colorBlend;
            struct DynamicState
            {
                std::vector<VkDynamicState> dynamicStates = {
                    VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT,
                    VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT,
                    VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
                    VK_DYNAMIC_STATE_LINE_WIDTH,
                };
            } dynamic;
        };

        std::shared_ptr<Device> _device;
        std::shared_ptr<RenderPass> _renderpass;
        VkPipelineLayout _pipelineLayout;
        VkPipeline _pipeline;
        uint32_t _subpassIndex;
        std::vector<VkPipelineShaderStageCreateInfo> stages;
        std::vector<VkVertexInputBindingDescription> vertexInputBindings;
        std::vector<VkVertexInputAttributeDescription> vertexInputAttributes;

        template <typename T>
        void AddVertexBindingDescription(uint32_t binding)
        {
            VkVertexInputBindingDescription bindingDescription = {
                .binding = binding,
                .stride = sizeof(T),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            };
            vertexInputBindings.emplace_back(bindingDescription);
        }

        void AddVertexAttribute(uint32_t binding, uint32_t location, uint32_t offset, int format);

        void AddShaderStage(Shader &shader);

        GraphicsPipeline(const std::shared_ptr<Device> &device,
                         const std::shared_ptr<PipelineLayout> &connector,
                         const std::shared_ptr<RenderPass> &renderpass);
        // maybe a build() or construct() ?
        //  solution: Assemble(...)
        ~GraphicsPipeline();

        void Assemble(uint32_t subpassIndex, const PipelineBuildData &pipelineData);
    };
}