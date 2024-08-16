//
// Created by piero on 3/08/2024
//
#pragma once
#include <iostream>
#include <volk.h>
#include <vector>
#include <string>
#include "Device.hpp"
#include "Shader.hpp"
#include "Swapchain.hpp"
#include "Utils.hpp"

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

        void AddShaderStage(Shader &shader);

        void Build(Device &device, PipelineConnector &connector, Swapchain &swapchain, FrontFace frontFace = FrontFace::Clockwise);

        void Destroy();
    };
}