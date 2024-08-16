//
// Created by piero on 23/06/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include <volk.h>
#include "Device.hpp"
#include "DescriptorLayout.hpp"

namespace Lettuce::Core
{
    class PipelineConnector
    {
    public:
        Device _device;
        VkPipelineLayout _pipelineLayout;
        std::vector<VkPushConstantRange> pushConstants;
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;

        void AddDescriptor(DescriptorLayout &descriptor);

        template <typename T1>
        void AddPushConstant(uint32_t offset, PipelineStage stage);

        void Build(Device &device);

        void Destroy();
    };
}