//
// Created by piero on 14/02/2024.
//
#pragma once
#include <iostream>
#include "Device.hpp"
#include "PipelineLayout.hpp"
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

        void Build(Device &device, PipelineLayout &connector, Shader &shader);

        void Destroy();
    };
}