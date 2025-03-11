//
// Created by piero on 6/03/2025.
//
#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "Device.hpp"
#include "IReleasable.hpp"
#include "Utils.hpp"
#include "PipelineLayout.hpp"

namespace Lettuce::Core
{
    class IndirectCommandsLayout : public IReleasable
    {
        uint32_t currentOffset = 0;
        uint32_t size = 0;
        std::vector<VkIndirectCommandsLayoutTokenEXT> tokens;

    public:
        VkShaderStageFlags _shaderStages;
        VkIndirectCommandsLayoutEXT _commandsLayout;
        std::shared_ptr<Device> _device;
        std::shared_ptr<PipelineLayout> _pipelineLayout;

        IndirectCommandsLayout(const std::shared_ptr<Device> &device, const std::shared_ptr<PipelineLayout> &pipelineLayout)
            : _device(device), _pipelineLayout(pipelineLayout) {}
        void Assemble(VkShaderStageFlags shaderStages, bool manualPreprocessing);
        void Release();

        uint32_t GetSize() { return size; }

        // Common Tokens
        void AddExecutionSetToken(VkShaderStageFlags shaderStages, uint32_t size);                // VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT
        void AddPushConstantToken(VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size); // VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT
        void AddSequenceToken(uint32_t size);                                                     // VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT
        // Compute Tokens
        void AddDispatchToken(); // VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_EXT
        // Graphics State Tokens
        void AddIndexBufferToken();                        // VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_EXT
        void AddVertexBufferToken(uint32_t vertexBinding); // VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT
        // Graphics Draw Tokens
        void AddDrawIndexedToken();   // VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_EXT
        void AddDrawToken();          // VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT
        void AddDrawMeshTasksToken(); // VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_EXT
        // Graphics Draw Count Tokens
        void AddDrawIndexedCountToken();   // VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_COUNT_EXT
        void AddDrawCountToken();          // VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_COUNT_EXT
        void AddDrawMeshTasksCountToken(); // VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_EXT
    };
}