//
// Created by piero on 3/08/2024
//
#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <memory>
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
        std::shared_ptr<Device> _device;
        VkPipelineLayout _pipelineLayout;
        VkPipeline _pipeline;
        std::vector<VkPipelineShaderStageCreateInfo> stages;
        RenderPass _renderpass;
        uint32_t _subpassIndex;

        void AddShaderStage(Shader &shader);

        MeshPipeline(const std::shared_ptr<Device> &device, PipelineLayout &connector, RenderPass &renderpass, uint32_t subpassIndex, FrontFace frontFace = FrontFace::Clockwise);

        ~MeshPipeline();
    };
}