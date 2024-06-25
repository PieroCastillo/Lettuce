//
// Created by piero on 14/02/2024.
//
module;
#include <iostream>
#include <vector>
#define VOLK_IMPLEMENTATION
#include <volk.h>

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

        void AddShaderStage(Shader shader)
        {
            VkPipelineShaderStageCreateInfo pipelineShaderStageCI = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = (VkShaderStageFlagBits)shader._stage,
                .module = shader._shaderModule,
                // const char*                         pName;
            };
            stages.emplace_back(pipelineShaderStageCI);
        }

        void Create(Device &device, PipelineConnector &connector, Swapchain &swapchain)
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
            };
            VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            };
            VkPipelineViewportStateCreateInfo viewportStateCI = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            };
            VkPipelineRasterizationStateCreateInfo rasterizationStateCI = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            };
            VkPipelineMultisampleStateCreateInfo multisampleStateCI = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            };
            VkPipelineDepthStencilStateCreateInfo depthStencilStateCI = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            };
            VkPipelineColorBlendStateCreateInfo colorBlendStateCI = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
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
                // const VkPipelineDynamicStateCreateInfo*          pDynamicState;
                .layout = _pipelineLayout,
                .renderPass = nullptr,
                // uint32_t                                         subpass;
                // VkPipeline                                       basePipelineHandle;
                // int32_t                                          basePipelineIndex;
            };

            checkResult(vkCreateGraphicsPipelines(_device._device, VK_NULL_HANDLE, 1, &graphicsPipelineCI, nullptr, &_pipeline), "graphics pipeline created sucessfully");
        }
    };
}