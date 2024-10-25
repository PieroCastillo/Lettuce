//
// Created by piero on 14/02/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include <vector>
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
                VkCullModeFlags cullMode;
                VkFrontFace frontFace;
                VkBool32 depthBiasEnable;
                float depthBiasConstantFactor;
                float depthBiasClamp;
                float depthBiasSlopeFactor;
                float lineWidth;
            } rasterization;
            struct MultisampleState
            {
                VkSampleCountFlagBits rasterizationSamples;
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
            struct ColorBlendAttachmentState
            {
                VkBool32 blendEnable;
                VkBlendFactor srcColorBlendFactor;
                VkBlendFactor dstColorBlendFactor;
                VkBlendOp colorBlendOp;
                VkBlendFactor srcAlphaBlendFactor;
                VkBlendFactor dstAlphaBlendFactor;
                VkBlendOp alphaBlendOp;
                VkColorComponentFlags colorWriteMask;
            } colorBlendAttachment;
            struct ColorBlendState
            {
                VkBool32 logicOpEnable;
                VkLogicOp logicOp;
                uint32_t attachmentCount;
                VkPipelineColorBlendAttachmentState *pAttachments;
                float blendConstants[4];
            } colorBlend;
            struct DynamicState
            {
                std::vector<VkDynamicState> dynamicStates;
            } dynamic;
        };

        Device _device;
        VkPipelineLayout _pipelineLayout;
        VkPipeline _pipeline;
        RenderPass _renderpass;
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

        void Build(Device &device, PipelineLayout &connector, RenderPass &renderpass, uint32_t subpassIndex, FrontFace frontFace = FrontFace::Clockwise);

        void Destroy();
    };
}