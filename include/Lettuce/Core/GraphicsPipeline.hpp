//
// Created by piero on 14/02/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include "Device.hpp"
#include "Shader.hpp"
#include "Swapchain.hpp"
#include "Utils.hpp"
#include "PipelineConnector.hpp"

namespace Lettuce::Core
{
    // TODO: implement PipelineCache, ShaderStages, DynamicState
    class GraphicsPipeline
    {
    public:
        Device _device;
        VkPipelineLayout _pipelineLayout;
        VkPipeline _pipeline;
        std::vector<VkPipelineShaderStageCreateInfo> stages;
        std::vector<VkVertexInputBindingDescription> vertexInputBindings;
        std::vector<VkVertexInputAttributeDescription> vertexInputAttributes;

        template <typename T>
        void AddVertexBindingDescription(uint32_t binding);

        void AddVertexAttribute(uint32_t binding, uint32_t location, uint32_t offset, int format);

        void AddShaderStage(Shader &shader);

        void Build(Device &device, PipelineConnector &connector, Swapchain &swapchain, FrontFace frontFace = FrontFace::Clockwise);

        void Destroy();
    };
}