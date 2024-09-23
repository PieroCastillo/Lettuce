//
// Created by piero on 14/02/2024.
//
#pragma once
#include <iostream>
#include "Device.hpp"
#include "PipelineConnector.hpp"
#include "Swapchain.hpp"
#include "Shader.hpp"

namespace Lettuce::Core
{
    class ComputePipeline
    {
    public:
        Device _device;
        VkPipelineLayout _pipelineLayout;
        VkPipeline _pipeline;

        void Build(Device &device, PipelineConnector &connector, Shader &shader);

        void Destroy();
    };
}