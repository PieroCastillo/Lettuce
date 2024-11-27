//
// Created by piero on 23/06/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include "Device.hpp"
#include "Descriptors.hpp"

namespace Lettuce::Core
{
    class PipelineLayout
    {
    public:
        Device _device;
        VkPipelineLayout _pipelineLayout;
        std::vector<VkPushConstantRange> pushConstants;

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

        void Build(Device &device, Descriptors &descriptor);
        void Build(Device &device);

        void Destroy();
    };
}