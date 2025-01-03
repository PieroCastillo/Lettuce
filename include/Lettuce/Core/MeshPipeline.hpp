//
// Created by piero on 3/08/2024
//
#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "Device.hpp"
#include "Shader.hpp"
#include "RenderPass.hpp"
#include "Utils.hpp"
#include "PipelineLayout.hpp"

namespace Lettuce::Core
{
    /// @brief This is a Pipeline that allow use Mesh Rendering 
    class MeshPipeline
    {
    public:
        Device _device;
        VkPipelineLayout _pipelineLayout;
        VkPipeline _pipeline;
        std::vector<VkPipelineShaderStageCreateInfo> stages;
        RenderPass _renderpass;
        uint32_t _subpassIndex;

        void AddShaderStage(Shader &shader);

        void Build(Device &device, PipelineLayout &connector, RenderPass &renderpass, uint32_t subpassIndex, FrontFace frontFace = FrontFace::Clockwise);

        void Destroy();
    };
}