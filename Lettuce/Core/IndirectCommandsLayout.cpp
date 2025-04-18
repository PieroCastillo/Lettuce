//
// Created by piero on 6/03/2025.
//
#include "Lettuce/Core/Common.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/PipelineLayout.hpp"
#include "Lettuce/Core/IndirectCommandsLayout.hpp"

using namespace Lettuce::Core;

void IndirectCommandsLayout::Assemble(VkShaderStageFlags shaderStages, bool manualPreprocessing)
{
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkIndirectCommandsTokenTypeEXT.html token-command relation

    if(!_device->GetEnabledRecommendedFeatures().deviceGeneratedCommands)
    {
        throw std::runtime_error("Device Generated Commands MUST be supported to use IndirectCommandsLayout");
        std::abort();
    }

    _shaderStages = shaderStages;
    size = currentOffset;
    VkIndirectCommandsLayoutCreateInfoEXT layoutCI = {
        .sType = VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_CREATE_INFO_EXT,
        .flags = manualPreprocessing ? VK_INDIRECT_COMMANDS_LAYOUT_USAGE_EXPLICIT_PREPROCESS_BIT_EXT : (VkIndirectCommandsLayoutUsageFlagsEXT)0,
        .shaderStages = shaderStages,
        .indirectStride = 0,
        .pipelineLayout = _pipelineLayout->GetHandle(),
        .tokenCount = (uint32_t)tokens.size(),
        .pTokens = tokens.data(),
    };

    checkResult(vkCreateIndirectCommandsLayoutEXT(_device->GetHandle(), &layoutCI, nullptr, GetHandlePtr()));
}

void IndirectCommandsLayout::Release()
{
    vkDestroyIndirectCommandsLayoutEXT(_device->GetHandle(), GetHandle(), nullptr);
}

void IndirectCommandsLayout::AddExecutionSetToken(VkShaderStageFlags shaderStages, uint32_t size)
{
    executionSetData = {VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT, shaderStages};

    VkIndirectCommandsTokenDataEXT data;
    data.pExecutionSet = &executionSetData;

    tokens.push_back({
        .sType = VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_EXT,
        .type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT,
        .data = data,
        .offset = currentOffset,
    });
    currentOffset += (4 + size) & ~3;
}

void IndirectCommandsLayout::AddPushConstantToken(VkShaderStageFlags stageFlags, uint32_t size)
{
    pushConstantDatas.push_back({stageFlags, pushCurrentOffset, size});
    pushCurrentOffset += size;
    VkIndirectCommandsTokenDataEXT data;
    data.pPushConstant = &(pushConstantDatas.data()[pushConstantDatas.size() - 1]);

    tokens.push_back({
        .sType = VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_EXT,
        .type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT,
        .data = data,
        .offset = currentOffset,
    });
    currentOffset += (4 + size) & ~3;
}

void IndirectCommandsLayout::AddSequenceToken(uint32_t size)
{
    tokens.push_back({
        .sType = VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_EXT,
        .type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT,
        .offset = currentOffset,
    });
    currentOffset += (4 + size) & ~3;
}

void IndirectCommandsLayout::AddDispatchToken()
{
    tokens.push_back({
        .sType = VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_EXT,
        .type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_EXT,
        .offset = currentOffset,
    });
    currentOffset += (4 + sizeof(VkDispatchIndirectCommand)) & ~3;
}

void IndirectCommandsLayout::AddIndexBufferToken()
{
    indexData = {VK_INDIRECT_COMMANDS_INPUT_MODE_VULKAN_INDEX_BUFFER_EXT};
    VkIndirectCommandsTokenDataEXT data;
    data.pIndexBuffer = &indexData;

    tokens.push_back({
        .sType = VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_EXT,
        .type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_EXT,
        .data = data,
        .offset = currentOffset,
    });
    currentOffset += (4 + sizeof(VkBindIndexBufferIndirectCommandEXT)) & ~3;
}

void IndirectCommandsLayout::AddVertexBufferToken(uint32_t vertexBinding)
{
    vertexDatas.push_back({vertexBinding});

    VkIndirectCommandsTokenDataEXT data;
    data.pVertexBuffer = &(vertexDatas.data()[vertexDatas.size() - 1]);

    tokens.push_back({
        .sType = VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_EXT,
        .type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT,
        .data = data,
        .offset = currentOffset,
    });
    currentOffset += (4 + sizeof(VkBindVertexBufferIndirectCommandEXT) & ~(4 - 1)); // minimum multiple of 4 major than ...
}

void IndirectCommandsLayout::AddDrawIndexedToken()
{
    tokens.push_back({
        .sType = VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_EXT,
        .type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_COUNT_EXT,
        .offset = currentOffset,
    });
    currentOffset += (4 + sizeof(VkDrawIndexedIndirectCommand)) & ~3;
}

void IndirectCommandsLayout::AddDrawToken()
{
    tokens.push_back({
        .sType = VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_EXT,
        .type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_COUNT_EXT,
        .offset = currentOffset,
    });
    currentOffset += (4 + sizeof(VkDrawIndirectCommand)) & ~3;
}

void IndirectCommandsLayout::AddDrawMeshTasksToken()
{
    tokens.push_back({
        .sType = VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_EXT,
        .type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_EXT,
        .offset = currentOffset,
    });
    currentOffset += (4 + sizeof(VkDrawMeshTasksIndirectCommandEXT)) & ~3;
}

void IndirectCommandsLayout::AddDrawIndexedCountToken()
{
    tokens.push_back({
        .sType = VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_EXT,
        .type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_COUNT_EXT,
        .offset = currentOffset,
    });
    currentOffset += (4 + sizeof(VkDrawIndirectCountIndirectCommandEXT) + sizeof(VkDrawIndexedIndirectCommand)) & ~3;
}

void IndirectCommandsLayout::AddDrawCountToken()
{
    tokens.push_back({
        .sType = VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_EXT,
        .type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_COUNT_EXT,
        .offset = currentOffset,
    });
    currentOffset += (4 + sizeof(VkDrawIndirectCountIndirectCommandEXT) + sizeof(VkDrawIndirectCommand)) & ~3;
}

void IndirectCommandsLayout::AddDrawMeshTasksCountToken()
{
    tokens.push_back({
        .sType = VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_EXT,
        .type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_EXT,
        .offset = currentOffset,
    });
    currentOffset += (4 + sizeof(VkDrawIndirectCountIndirectCommandEXT) + sizeof(VkDrawMeshTasksIndirectCommandEXT)) & ~3;
}