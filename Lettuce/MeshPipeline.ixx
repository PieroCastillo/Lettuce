//
// Created by piero on 3/08/2024
//
module;
#include <iostream>
#define VOLK_IMPLEMENTATION
#include <volk.h>
#include <vector>
#include <string>

export module Lettuce:MeshPipeline;

import :Device;
import :Utils;

export namespace Lettuce::Core
{
    /// @brief This is a Pipeline that allow use Mesh Rendering 
    class MeshPipeline
    {
    public:
        Device _device;
        VkPipelineLayout _pipelineLayout;
        VkPipeline _pipeline;
        std::vector<VkPipelineShaderStageCreateInfo> stages;

        void AddShaderStage(Shader &shader)
        {
            VkPipelineShaderStageCreateInfo pipelineShaderStageCI = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = (VkShaderStageFlagBits)shader._stage,
                .module = shader._shaderModule,
                .pName = shader._name.c_str(),
            };
            if(shader._stage != PipelineStage::Mesh | shader._stage != PipelineStage::Task | shader._stage != PipelineStage::Fragment ){
                throw new std::exception("Mesh Pipeline support Task|Mesh|Fragment shaders only");
            }
            stages.emplace_back(pipelineShaderStageCI);
        }

        void Build(Device &device, PipelineConnector &connector, Swapchain &swapchain, FrontFace frontFace = FrontFace::Clockwise)
        {
            _device = device;
            _pipelineLayout = connector._pipelineLayout;

            // required for dynamic rendering
            const VkPipelineRenderingCreateInfoKHR pipelineRenderingCI {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &swapchain.imageFormat,
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
                .pVertexInputState = nullptr,
                .pInputAssemblyState = nullptr,
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

            checkResult(vkCreateGraphicsPipelines(_device._device, VK_NULL_HANDLE, 1, &graphicsPipelineCI, nullptr, &_pipeline), "mesh pipeline created sucessfully");
        }

        void Destroy()
        {
            vkDestroyPipeline(_device._device, _pipeline, nullptr);
        }
    };
}