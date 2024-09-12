//
// Created by piero on 23/06/2024.
//
#pragma once
#include <iostream>
#include <vector>
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
        void AddPushConstant(uint32_t offset, PipelineStage stage)
        {
            VkPushConstantRange pushConstantRange = {
                .stageFlags = (VkShaderStageFlags)stage,
                .offset = offset,
                .size = sizeof(T1),
            };
            pushConstants.emplace_back(pushConstantRange);
        }

        void Build(Device &device);

        void Destroy();
    };
}