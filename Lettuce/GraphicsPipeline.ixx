//
// Created by piero on 14/02/2024.
//
module;
#include <iostream>
#include <vector>
#include "volk.h"
export module Lettuce:GraphicsPipeline;

import :Device;
import :Utils;
import :PipelineConnector;
import :Swapchain;
import :Shader;

export namespace Lettuce::Core
{
    // TODO: implement PipelineCache, ShaderStages, DynamicState
    class GraphicsPipeline
    {
    public:
        Device _device;
        VkPipelineLayout _pipelineLayout;
        VkPipeline _pipeline;
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

        void AddVertexAttribute(uint32_t binding, uint32_t location, uint32_t offset, int format)
        {
            VkVertexInputAttributeDescription attributeDescription = {
                .location = location,
                .binding = binding,
                .format = (VkFormat)format,
                .offset = offset,
            };
            vertexInputAttributes.emplace_back(attributeDescription);
        }

        void AddShaderStage(Shader &shader)
        {
            VkPipelineShaderStageCreateInfo pipelineShaderStageCI = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = (VkShaderStageFlagBits)shader._stage,
                .module = shader._shaderModule,
                .pName = shader._name.c_str(),
            };
            stages.emplace_back(pipelineShaderStageCI);
        }

        void Build(Device &device, PipelineConnector &connector, Swapchain &swapchain, FrontFace frontFace = FrontFace::Clockwise)
        {
            _device = device;
            _pipelineLayout = connector._pipelineLayout;

            // required for dynamic rendering
            const VkPipelineRenderingCreateInfoKHR pipelineRenderingCI{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &swapchain.imageFormat,
            };

            VkPipelineVertexInputStateCreateInfo vertexInputStateCI = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .vertexBindingDescriptionCount = 0,
                .vertexAttributeDescriptionCount = 0,
            };

            if (vertexInputAttributes.size() > 0)
            {
                vertexInputStateCI.vertexAttributeDescriptionCount = (uint32_t)vertexInputAttributes.size();
                vertexInputStateCI.pVertexAttributeDescriptions = vertexInputAttributes.data();
            }

            if (vertexInputBindings.size() > 0)
            {
                vertexInputStateCI.vertexBindingDescriptionCount = (uint32_t)vertexInputBindings.size();
                vertexInputStateCI.pVertexBindingDescriptions = vertexInputBindings.data();
            }

            VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            };
            VkPipelineViewportStateCreateInfo viewportStateCI = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .viewportCount = 1,
                .scissorCount = 1,
            };
            VkPipelineRasterizationStateCreateInfo rasterizationStateCI = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                .cullMode = VK_CULL_MODE_BACK_BIT,
                .frontFace = (VkFrontFace)frontFace,
            };
            VkPipelineMultisampleStateCreateInfo multisampleStateCI = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            };
            VkPipelineDepthStencilStateCreateInfo depthStencilStateCI = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            };

            VkPipelineColorBlendAttachmentState attachment = {
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            };

            VkPipelineColorBlendStateCreateInfo colorBlendStateCI = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .attachmentCount = 1,
                .pAttachments = &attachment,
            };

            std::vector<VkDynamicState> dynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR,
                VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
                VK_DYNAMIC_STATE_LINE_WIDTH,
            };
            VkPipelineDynamicStateCreateInfo dynamicStateCI = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                .dynamicStateCount = (uint32_t)dynamicStates.size(),
                .pDynamicStates = dynamicStates.data(),
            };

            VkGraphicsPipelineCreateInfo graphicsPipelineCI = {
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .pNext = &pipelineRenderingCI,
                //                 VkPipelineCreateFlags                            flags;
                .stageCount = (uint32_t)stages.size(),
                .pStages = stages.data(),
                .pVertexInputState = &vertexInputStateCI,
                .pInputAssemblyState = &inputAssemblyStateCI,
                // const VkPipelineTessellationStateCreateInfo*     pTessellationState;
                .pViewportState = &viewportStateCI,
                .pRasterizationState = &rasterizationStateCI,
                .pMultisampleState = &multisampleStateCI,
                .pDepthStencilState = &depthStencilStateCI,
                .pColorBlendState = &colorBlendStateCI,
                .pDynamicState = &dynamicStateCI,
                .layout = _pipelineLayout,
                .renderPass = nullptr,
                // uint32_t                                         subpass;
                // VkPipeline                                       basePipelineHandle;
                // int32_t                                          basePipelineIndex;
            };

            checkResult(vkCreateGraphicsPipelines(_device._device, VK_NULL_HANDLE, 1, &graphicsPipelineCI, nullptr, &_pipeline), "graphics pipeline created sucessfully");
        }

        void Destroy()
        {
            vkDestroyPipeline(_device._device, _pipeline, nullptr);
        }
    };
}