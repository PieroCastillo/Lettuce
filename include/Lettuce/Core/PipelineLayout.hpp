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

        void Build(const std::shared_ptr<Device> &device, Descriptors &descriptor);
        void Build(const std::shared_ptr<Device> &device);

        void Destroy();
    };
}