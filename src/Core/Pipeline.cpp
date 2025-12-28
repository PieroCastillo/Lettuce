// standard headers
#include <memory>
#include <array>
#include <print>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Core/common.hpp"

using namespace Lettuce::Core;

// void Pipeline::Create(const IDevice& device, const GraphicsPipelineCreateInfo& createInfo)
// {
//     m_device = device.m_device;
//     m_layout = createInfo.layout;

//     VkPipelineViewportStateCreateInfo viewportState =
//     {
//         .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
//         .viewportCount = 1,
//         .scissorCount = 1,
//     };
//     VkPipelineRasterizationStateCreateInfo rasterizationState =
//     {
//         .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
//         .depthClampEnable = VK_FALSE,
//         .rasterizerDiscardEnable = VK_FALSE,
//         .polygonMode = VK_POLYGON_MODE_FILL,
//         .cullMode = VK_CULL_MODE_FRONT_BIT,
//         .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
//         //.lineWidth // dynamic;
//     };
//     VkPipelineMultisampleStateCreateInfo multisampleState =
//     {
//         .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
//         .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
//         .sampleShadingEnable = VK_FALSE,
//     };
//     VkPipelineDepthStencilStateCreateInfo depthStencilState =
//     {
//         .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
//         .depthTestEnable = VK_FALSE,
//         .depthWriteEnable = VK_FALSE,
//     };

//     VkPipelineColorBlendAttachmentState colorBlendAttachment = {
//         .blendEnable = VK_FALSE,
//         .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
//     };

//     std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(createInfo.colorAttachmentFormats.size(), colorBlendAttachment);

//     // blending disabled, pipeline writes "directly" to the attachments
//     VkPipelineColorBlendStateCreateInfo colorBlendState =
//     {
//         .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
//         .logicOpEnable = VK_FALSE,
//         .attachmentCount = (uint32_t)colorBlendAttachments.size(),
//         .pAttachments = colorBlendAttachments.data(),
//     };

//     std::array<VkDynamicState, 3> dynamicStates =
//     {
//         VK_DYNAMIC_STATE_VIEWPORT,
//         VK_DYNAMIC_STATE_SCISSOR,
//         VK_DYNAMIC_STATE_LINE_WIDTH,
//     };

//     VkPipelineDynamicStateCreateInfo dynamicState =
//     {
//         .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
//         .dynamicStateCount = (uint32_t)dynamicStates.size(),
//         .pDynamicStates = dynamicStates.data(),
//     };

//     VkPipelineFragmentShadingRateStateCreateInfoKHR fragmentShadingRate = {
//         .sType = VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_STATE_CREATE_INFO_KHR,
//     };

//     // TODO: impl Depth Testing
//     VkPipelineRenderingCreateInfo renderingCI = {
//         .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
//         .pNext = createInfo.fragmentShadingRate ? &fragmentShadingRate : nullptr,
//         .viewMask = 0, // multiview disabled
//         .colorAttachmentCount = (uint32_t)createInfo.colorAttachmentFormats.size(),
//         .pColorAttachmentFormats = createInfo.colorAttachmentFormats.data(),
//         //.depthAttachmentFormat = createInfo.depthAttachmentFormat,
//         //.stencilAttachmentFormat = createInfo.stencilAttachmentFormat,
//     };

//     std::vector<VkPipelineShaderStageCreateInfo> stages;
//     stages.reserve(createInfo.stages.size());

//     for (int i = 0; i < createInfo.shaderModules.size(); ++i)
//     {
//         VkPipelineShaderStageCreateInfo stageCI = {
//             .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
//             .stage = createInfo.stages[i],
//             .module = createInfo.shaderModules[i],
//             .pName = createInfo.entryPoints[i].c_str(),
//             // TODO: impl specialization constants
//         };
//         stages.push_back(std::move(stageCI));
//     }

//     VkGraphicsPipelineCreateInfo gpipelineCI = {
//         .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
//         .pNext = &renderingCI,
//         .flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
//         // stages
//         .stageCount = (uint32_t)stages.size(),
//         .pStages = stages.data(),
//         .pViewportState = &viewportState,
//         .pRasterizationState = &rasterizationState,
//         .pMultisampleState = &multisampleState,
//         .pDepthStencilState = &depthStencilState,
//         .pColorBlendState = &colorBlendState,
//         .pDynamicState = &dynamicState,
//         .layout = m_layout,
//         .subpass = 0,
//         .basePipelineHandle = VK_NULL_HANDLE,
//     };

//     if (!createInfo.useMeshShader)
//     {
//         // vertex input state
//         gpipelineCI.pVertexInputState = &vertexInputState;
//         gpipelineCI.pInputAssemblyState = &inputAssemblyState;
//         // pre rasterization shader state
//         gpipelineCI.pTessellationState = &tessellationState;
//     }

//     handleResult(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &gpipelineCI, nullptr, &m_pipeline));
// }

Pipeline Device::CreatePipeline(const PrimitiveShadingPipelineDesc& desc)
{
    return impl->pipelines.allocate({});
}

Pipeline Device::CreatePipeline(const MeshShadingPipelineDesc& desc)
{
    return impl->pipelines.allocate({});
}

Pipeline Device::CreatePipeline(const ComputePipelineDesc& desc)
{
    // VkPipelineShaderStageCreateInfo stageCI = {
    //     .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    //     .stage = VK_SHADER_STAGE_COMPUTE_BIT,
    //     .module = desc.shaderModule,
    //     .pName = desc.entryPoint.c_str(),
    // };

    // VkComputePipelineCreateInfo cpipelineCI = {
    //     .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
    //     .flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
    //     .stage = stageCI,
    //     .layout = createInfo.layout,
    // };
    // handleResult(vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &cpipelineCI, nullptr, &m_pipeline));
    return impl->pipelines.allocate({});
}

void Device::Destroy(Pipeline pipeline)
{
    auto vkPipeline = impl->pipelines.get(pipeline);
    vkDestroyPipeline(impl->m_device, vkPipeline, nullptr);
    impl->pipelines.free(pipeline);
}