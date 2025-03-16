//
// Created by piero on 6/03/2025.
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include "Lettuce/Core/Shader.hpp"
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/IndirectCommandsLayout.hpp"
#include "Lettuce/Core/IndirectExecutionSet.hpp"

using namespace Lettuce::Core;

void IndirectExecutionSet::Assemble(const std::vector<std::shared_ptr<Shader>> &initialShaders, uint32_t maxShaderCount)
{
    std::vector<VkIndirectExecutionSetShaderLayoutInfoEXT> setLayouts(initialShaders.size(), {
        .sType = VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_SHADER_LAYOUT_INFO_EXT,
        .setLayoutCount = (uint32_t)_pipelineLayout->_descriptors->_layouts.size(),
        .pSetLayouts = _pipelineLayout->_descriptors->_layouts.data(),
    });

    // get shaders
    std::vector<VkShaderEXT> shaders;
    shaders.reserve(initialShaders.size());

    for (auto &shaderPtr : initialShaders)
    {
        shaders.push_back(shaderPtr->_shader);
    }

    VkIndirectExecutionSetShaderInfoEXT shadersInfo = {
        .sType = VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_SHADER_INFO_EXT,
        .shaderCount = (uint32_t)initialShaders.size(),
        .pInitialShaders = shaders.data(),
        .pSetLayoutInfos = setLayouts.data(),
        .maxShaderCount = (uint32_t)maxShaderCount,
        .pushConstantRangeCount = (uint32_t)_pipelineLayout->pushConstants.size(),
        .pPushConstantRanges = _pipelineLayout->pushConstants.data(),
    };

    VkIndirectExecutionSetInfoEXT info;
    info.pShaderInfo = &shadersInfo;

    VkIndirectExecutionSetCreateInfoEXT executionSetCI = {
        .sType = VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_CREATE_INFO_EXT,
        .type = VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT,
        .info = info,
    };
    checkResult(vkCreateIndirectExecutionSetEXT(_device->_device, &executionSetCI, nullptr, &_executionSet));
}

void IndirectExecutionSet::Update(const std::vector<std::shared_ptr<Shader>> &shaders)
{
    std::vector<VkWriteIndirectExecutionSetShaderEXT> shaderWrites;
    shaderWrites.resize(shaders.size());

    for (int i = 0; i < shaderWrites.size(); i++)
    {
        shaderWrites[i] ={
            .sType = VK_STRUCTURE_TYPE_WRITE_INDIRECT_EXECUTION_SET_SHADER_EXT,
            .index = (uint32_t)i,
            .shader = shaders[i]->_shader,
        };
    }
    vkUpdateIndirectExecutionSetShaderEXT(_device->_device, _executionSet, (uint32_t)shaderWrites.size(), shaderWrites.data());
}

void IndirectExecutionSet::Release()
{
    vkDestroyIndirectExecutionSetEXT(_device->_device, _executionSet, nullptr);
}

VkMemoryRequirements IndirectExecutionSet::GetRequirements(const std::shared_ptr<IndirectCommandsLayout>& layout, uint32_t maxSequenceCount, uint32_t maxDrawCount)
{
    VkGeneratedCommandsMemoryRequirementsInfoEXT info = {
        .sType = VK_STRUCTURE_TYPE_GENERATED_COMMANDS_MEMORY_REQUIREMENTS_INFO_EXT,
        .indirectExecutionSet = _executionSet,
        .indirectCommandsLayout = layout->_commandsLayout,
        .maxSequenceCount = maxSequenceCount,
        .maxDrawCount = maxDrawCount,
    };
    VkMemoryRequirements2 reqs = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
    };
    vkGetGeneratedCommandsMemoryRequirementsEXT(_device->_device, &info, &reqs);
    return reqs.memoryRequirements;
}