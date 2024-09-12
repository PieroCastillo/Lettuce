//
// Created by piero on 14/02/2024.
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <vector>
#include "Lettuce/Core/GraphicsPipeline.hpp"
#include "Lettuce/Core/PipelineConnector.hpp"
#include "Lettuce/Core/Swapchain.hpp"
#include "Lettuce/Core/Shader.hpp"

using namespace Lettuce::Core;

void GraphicsPipeline::AddVertexAttribute(uint32_t binding, uint32_t location, uint32_t offset, int format)
{
    VkVertexInputAttributeDescription attributeDescription = {
        .location = location,
        .binding = binding,
        .format = (VkFormat)format,
        .offset = offset,
    };
    vertexInputAttributes.emplace_back(attributeDescription);
}

void GraphicsPipeline::AddShaderStage(Shader &shader)
{
    VkPipelineShaderStageCreateInfo pipelineShaderStageCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = (VkShaderStageFlagBits)shader._stage,
        .module = shader._shaderModule,
        .pName = shader._name.c_str(),
    };
    stages.emplace_back(pipelineShaderStageCI);
}

void GraphicsPipeline::Build(Device &device, PipelineConnector &connector, Swapchain &swapchain, FrontFace frontFace)
{
    _device = device;
    _pipelineLayout = connector._pipelineLayout;

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
        .renderPass = swapchain._renderPass,
        .subpass = 0,
        // VkPipeline                                       basePipelineHandle;
        // int32_t                                          basePipelineIndex;
    };

    checkResult(vkCreateGraphicsPipelines(_device._device, VK_NULL_HANDLE, 1, &graphicsPipelineCI, nullptr, &_pipeline), "graphics pipeline created sucessfully");
}

void GraphicsPipeline::Destroy()
{
    vkDestroyPipeline(_device._device, _pipeline, nullptr);
}