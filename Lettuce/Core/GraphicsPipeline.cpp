//
// Created by piero on 14/02/2024.
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <vector>
#include "Lettuce/Core/GraphicsPipeline.hpp"
#include "Lettuce/Core/PipelineLayout.hpp"
#include "Lettuce/Core/Swapchain.hpp"
#include "Lettuce/Core/Shader.hpp"
#include "Lettuce/Core/RenderPass.hpp"

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

void GraphicsPipeline::Assemble(uint32_t subpassIndex, const PipelineBuildData &pipelineData)
{
    _subpassIndex = subpassIndex;

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
        .topology = pipelineData.inputAssembly.topology,
        .primitiveRestartEnable = pipelineData.inputAssembly.primitiveRestartEnable,
    };
    VkPipelineViewportStateCreateInfo viewportStateCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        // .viewportCount = (uint32_t)pipelineData.viewport.viewports.size(),
        // .pViewports = pipelineData.viewport.viewports.data(),
        // .scissorCount = (uint32_t)pipelineData.viewport.scissors.size(),
        // .pScissors = pipelineData.viewport.scissors.data(),
    };
    if (pipelineData.viewport.viewports.size() > 0)
    {
        viewportStateCI.viewportCount = (uint32_t)pipelineData.viewport.viewports.size();
        viewportStateCI.pViewports = pipelineData.viewport.viewports.data();
    }
    if (pipelineData.viewport.scissors.size() > 0)
    {
        viewportStateCI.scissorCount = (uint32_t)pipelineData.viewport.scissors.size();
        viewportStateCI.pScissors = pipelineData.viewport.scissors.data();
    }
    VkPipelineRasterizationStateCreateInfo rasterizationStateCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = pipelineData.rasterization.depthBiasEnable,
        .rasterizerDiscardEnable = pipelineData.rasterization.rasterizerDiscardEnable,
        .polygonMode = pipelineData.rasterization.polygonMode,
        .cullMode = pipelineData.rasterization.cullMode,
        .frontFace = pipelineData.rasterization.frontFace,
        .depthBiasEnable = pipelineData.rasterization.depthBiasEnable,
        .depthBiasConstantFactor = pipelineData.rasterization.depthBiasConstantFactor,
        .depthBiasClamp = pipelineData.rasterization.depthBiasClamp,
        .depthBiasSlopeFactor = pipelineData.rasterization.depthBiasSlopeFactor,
        .lineWidth = pipelineData.rasterization.lineWidth,
    };
    VkPipelineMultisampleStateCreateInfo multisampleStateCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = pipelineData.multisample.rasterizationSamples,
        .sampleShadingEnable = pipelineData.multisample.sampleShadingEnable,
        .minSampleShading = pipelineData.multisample.minSampleShading,
        .pSampleMask = pipelineData.multisample.pSampleMask,
        .alphaToCoverageEnable = pipelineData.multisample.alphaToCoverageEnable,
        .alphaToOneEnable = pipelineData.multisample.alphaToOneEnable,
    };
    VkPipelineDepthStencilStateCreateInfo depthStencilStateCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = pipelineData.depthStencil.depthTestEnable,
        .depthWriteEnable = pipelineData.depthStencil.depthWriteEnable,
        .depthCompareOp = pipelineData.depthStencil.depthCompareOp,
        .depthBoundsTestEnable = pipelineData.depthStencil.depthBoundsTestEnable,
        .stencilTestEnable = pipelineData.depthStencil.stencilTestEnable,
        .front = pipelineData.depthStencil.front,
        .back = pipelineData.depthStencil.back,
        .minDepthBounds = pipelineData.depthStencil.minDepthBounds,
        .maxDepthBounds = pipelineData.depthStencil.maxDepthBounds,
    };

    VkPipelineColorBlendStateCreateInfo colorBlendStateCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = pipelineData.colorBlend.logicOpEnable,
        .logicOp = pipelineData.colorBlend.logicOp,
        .attachmentCount = (uint32_t)pipelineData.colorBlend.attachments.size(),
        .pAttachments = pipelineData.colorBlend.attachments.data(),
        .blendConstants = {
            pipelineData.colorBlend.blendConstants[0],
            pipelineData.colorBlend.blendConstants[1],
            pipelineData.colorBlend.blendConstants[2],
            pipelineData.colorBlend.blendConstants[3],
        },
    };

    VkPipelineDynamicStateCreateInfo dynamicStateCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = (uint32_t)pipelineData.dynamic.dynamicStates.size(),
        .pDynamicStates = pipelineData.dynamic.dynamicStates.data(),
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
        .renderPass = _renderpass->_renderPass,
        .subpass = _subpassIndex,
        // VkPipeline                                       basePipelineHandle;
        // int32_t                                          basePipelineIndex;
    };

    checkResult(vkCreateGraphicsPipelines(_device->_device, VK_NULL_HANDLE, 1, &graphicsPipelineCI, nullptr, &_pipeline));
}

GraphicsPipeline::GraphicsPipeline(const std::shared_ptr<Device> &device,
                                   const std::shared_ptr<PipelineLayout> &connector,
                                   const std::shared_ptr<RenderPass> &renderpass)
    : _device(device),
      _pipelineLayout(connector->_pipelineLayout),
      _renderpass(renderpass)
{
}

GraphicsPipeline::~GraphicsPipeline()
{
    vkDestroyPipeline(_device->_device, _pipeline, nullptr);
}