// standard headers

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/Pipeline.hpp"

using namespace Lettuce::Core;

LettuceResult Pipeline::Create(VkDevice device, const GraphicsPipelineCreateInfo& createInfo)
{

    VkPipelineVertexInputStateCreateInfo vertexInputState =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    };
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = createInfo.topology,
        .primitiveRestartEnable = VK_FALSE,
    };
    VkPipelineTessellationStateCreateInfo tessellationState =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
        .patchControlPoints = createInfo.patchControlPoints,
    };
    VkPipelineViewportStateCreateInfo viewportState =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    };
    VkPipelineRasterizationStateCreateInfo rasterizationState =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        //.lineWidth // dynamic;
    };
    VkPipelineMultisampleStateCreateInfo multisampleState =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
    };
    VkPipelineDepthStencilStateCreateInfo depthStencilState =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    };
    VkPipelineColorBlendStateCreateInfo colorBlendState =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    };

    std::array<VkDynamicState, 3> dynamicStates =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_LINE_WIDTH,
    };
    VkPipelineDynamicStateCreateInfo dynamicState =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = (uint32_t)dynamicStates.size(),
        .pDynamicStates = dynamicStates.count(),
    };

    VkPipelineFragmentShadingRateStateCreateInfoKHR fragmentShadingRate = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_STATE_CREATE_INFO_KHR,
    };

    VkPipelineRenderingCreateInfo renderingCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext = createInfo.fragmentShadingRate ? nullptr : &fragmentShadingRate,
        .viewMask = 0, // multiview disabled
        .colorAttachmentCount = ,
        .pColorAttachmentFormats = ,
        .depthAttachmentFormat = ,
        .stencilAttachmentFormat = ,
    };

    VkGraphicsPipelineCreateInfo gpipelineCI = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &renderingCI,
        // vertex input state
        .pVertexInputState = &vertexInputState,
        .pInputAssemblyState = &inputAssemblyState,
        // pre rasterization shader state
        .pTessellationState = &tessellationState,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizationState,
        .pMultisampleState = &multisampleState,
        .pDepthStencilState = &depthStencilState,
        .pColorBlendState = &colorBlendState,
        .pDynamicState = &dynamicState,
    };

    return LettuceResult::Success;
}

LettuceResult Pipeline::Create(VkDevice device, const ComputePipelineCreateInfo& createInfo)
{
    return LettuceResult::Success;
}

void Pipeline::Release()
{

}