//
// Created by piero on 14/02/2024.
//
#pragma once
#include <iostream>
#include <memory>
#include "Device.hpp"
#include "PipelineLayout.hpp"
#include "Swapchain.hpp"
#include "Shader.hpp"

namespace Lettuce::Core
{
    class ComputePipeline
    {
    public:
        std::shared_ptr<Device> _device;
        VkPipelineLayout _pipelineLayout;
        VkPipeline _pipeline;

        void Build(const std::shared_ptr<Device> &device, PipelineLayout &connector, Shader &shader);

        void Destroy();
    };
}