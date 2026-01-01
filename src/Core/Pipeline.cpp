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

Pipeline Device::CreatePipeline(const PrimitiveShadingPipelineDesc& desc)
{
    VkPipelineViewportStateCreateInfo viewportState =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };
    VkPipelineRasterizationStateCreateInfo rasterizationState =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_FRONT_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
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
        .depthTestEnable = VK_FALSE,
        .depthWriteEnable = VK_FALSE,
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(desc.colorAttachmentFormats.size(), colorBlendAttachment);

    // blending disabled, pipeline writes "directly" to the attachments
    VkPipelineColorBlendStateCreateInfo colorBlendState =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = (uint32_t)colorBlendAttachments.size(),
        .pAttachments = colorBlendAttachments.data(),
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
        .pDynamicStates = dynamicStates.data(),
    };

    VkPipelineFragmentShadingRateStateCreateInfoKHR fragmentShadingRate = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_STATE_CREATE_INFO_KHR,
    };

    std::vector<VkFormat> tmp;
    tmp.reserve(desc.colorAttachmentFormats.size());

    for (auto e : desc.colorAttachmentFormats) {
        tmp.push_back(ToVkFormat(e));
    }

    // TODO: impl Depth Testing
    VkPipelineRenderingCreateInfo renderingCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext = desc.fragmentShadingRate ? &fragmentShadingRate : nullptr,
        .viewMask = 0, // multiview disabled
        .colorAttachmentCount = (uint32_t)tmp.size(),
        .pColorAttachmentFormats = tmp.data(),
        .depthAttachmentFormat = ToVkFormat(desc.depthStencilAttachmentFormat),
        .stencilAttachmentFormat = ToVkFormat(desc.depthStencilAttachmentFormat),
    };

    auto& shaders = impl->shaders;
    std::array<VkPipelineShaderStageCreateInfo, 2> stages;
    stages[0] = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_MESH_BIT_EXT,
        .module = shaders.get(desc.vertShaderBinary),
        .pName = std::string(desc.vertEntryPoint).c_str(),
    };
    stages[1] = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = shaders.get(desc.fragShaderBinary),
        .pName = std::string(desc.fragEntryPoint).c_str(),
    };

    auto layout = impl->descriptorTables.get(desc.descriptorTable).pipelineLayout;
    VkGraphicsPipelineCreateInfo gpipelineCI = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &renderingCI,
        .flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
        // stages
        .stageCount = (uint32_t)stages.size(),
        .pStages = stages.data(),
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizationState,
        .pMultisampleState = &multisampleState,
        .pDepthStencilState = &depthStencilState,
        .pColorBlendState = &colorBlendState,
        .pDynamicState = &dynamicState,
        .layout = layout,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
    };

    VkPipeline pipeline;
    handleResult(vkCreateGraphicsPipelines(impl->m_device, VK_NULL_HANDLE, 1, &gpipelineCI, nullptr, &pipeline));
    return impl->pipelines.allocate({ pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS });
}

Pipeline Device::CreatePipeline(const MeshShadingPipelineDesc& desc)
{
    VkPipelineViewportStateCreateInfo viewportState =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };
    VkPipelineRasterizationStateCreateInfo rasterizationState =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_FRONT_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
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
        .depthTestEnable = VK_FALSE,
        .depthWriteEnable = VK_FALSE,
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(desc.colorAttachmentFormats.size(), colorBlendAttachment);

    // blending disabled, pipeline writes "directly" to the attachments
    VkPipelineColorBlendStateCreateInfo colorBlendState =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = (uint32_t)colorBlendAttachments.size(),
        .pAttachments = colorBlendAttachments.data(),
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
        .pDynamicStates = dynamicStates.data(),
    };

    VkPipelineFragmentShadingRateStateCreateInfoKHR fragmentShadingRate = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_STATE_CREATE_INFO_KHR,
    };

    std::vector<VkFormat> tmp;
    tmp.reserve(desc.colorAttachmentFormats.size());

    for (auto e : desc.colorAttachmentFormats) {
        tmp.push_back(ToVkFormat(e));
    }

    // TODO: impl Depth Testing
    VkPipelineRenderingCreateInfo renderingCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext = desc.fragmentShadingRate ? &fragmentShadingRate : nullptr,
        .viewMask = 0, // multiview disabled
        .colorAttachmentCount = (uint32_t)tmp.size(),
        .pColorAttachmentFormats = tmp.data(),
        .depthAttachmentFormat = ToVkFormat(desc.depthStencilAttachmentFormat),
        .stencilAttachmentFormat = ToVkFormat(desc.depthStencilAttachmentFormat),
    };

    auto& shaders = impl->shaders;
    std::vector<VkPipelineShaderStageCreateInfo> stages(2);
    stages[0] = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_MESH_BIT_EXT,
        .module = shaders.get(desc.meshShaderBinary),
        .pName = std::string(desc.meshEntryPoint).c_str(),
    };
    stages[1] = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = shaders.get(desc.fragShaderBinary),
        .pName = std::string(desc.fragEntryPoint).c_str(),
    };
    if (desc.taskShaderBinary != std::nullopt && desc.taskEntryPoint != std::nullopt)
    {
        stages.resize(3);
        stages[2] = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_TASK_BIT_EXT,
            .module = shaders.get(desc.taskShaderBinary.value()),
            .pName = std::string(desc.taskEntryPoint.value()).c_str(),
        };
    }
    auto layout = impl->descriptorTables.get(desc.descriptorTable).pipelineLayout;
    VkGraphicsPipelineCreateInfo gpipelineCI = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &renderingCI,
        .flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
        // stages
        .stageCount = (uint32_t)stages.size(),
        .pStages = stages.data(),
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizationState,
        .pMultisampleState = &multisampleState,
        .pDepthStencilState = &depthStencilState,
        .pColorBlendState = &colorBlendState,
        .pDynamicState = &dynamicState,
        .layout = layout,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
    };

    VkPipeline pipeline;
    handleResult(vkCreateGraphicsPipelines(impl->m_device, VK_NULL_HANDLE, 1, &gpipelineCI, nullptr, &pipeline));
    return impl->pipelines.allocate({ pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS });
}

Pipeline Device::CreatePipeline(const ComputePipelineDesc& desc)
{
    VkPipelineShaderStageCreateInfo stageCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = impl->shaders.get(desc.compShaderBinary),
        .pName = std::string(desc.compEntryPoint).c_str(),
    };

    VkComputePipelineCreateInfo cpipelineCI = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
        .stage = stageCI,
        .layout = impl->descriptorTables.get(desc.descriptorTable).pipelineLayout,
    };
    VkPipeline pipeline;
    handleResult(vkCreateComputePipelines(impl->m_device, VK_NULL_HANDLE, 1, &cpipelineCI, nullptr, &pipeline));
    return impl->pipelines.allocate({ pipeline, VK_PIPELINE_BIND_POINT_COMPUTE });
}

void Device::Destroy(Pipeline pipeline)
{
    auto vkPipeline = impl->pipelines.get(pipeline);
    vkDestroyPipeline(impl->m_device, vkPipeline.pipeline, nullptr);
    impl->pipelines.free(pipeline);
}