//
// Created by piero on 23/06/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include "Device.hpp"
#include "Descriptors.hpp"
#include <memory>

namespace Lettuce::Core
{
    class PipelineLayout
    {
    public:
        std::shared_ptr<Device> _device;
        VkPipelineLayout _pipelineLayout;

        PipelineLayout(const std::shared_ptr<Device> &device, const std::vector<VkPushConstantRange> &pushConstants, const std::shared_ptr<Descriptors> &descriptor);
        PipelineLayout(const std::shared_ptr<Device> &device);

        ~PipelineLayout();
    };
}