//
// Created by piero on 23/06/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include "Device.hpp"
#include "IReleasable.hpp"
#include "Descriptors.hpp"
#include <memory>
#include "IManageHandle.hpp"

namespace Lettuce::Core
{
    class PipelineLayout : public IReleasable, public IManageHandle<VkPipelineLayout>
    {
    private:
        uint32_t currentOffset = 0;

    public:
        std::shared_ptr<Device> _device;
        std::shared_ptr<Descriptors> _descriptors;
        std::vector<VkPushConstantRange> pushConstants;
        
        PipelineLayout(const std::shared_ptr<Device> &device);
        PipelineLayout(const std::shared_ptr<Device> &device, const std::shared_ptr<Descriptors> &descriptor);

        void Release();

        template <typename T1>
        void AddPushConstant(VkShaderStageFlags stages)
        {
            AddPushConstant(sizeof(T1), stages);
        }

        void AddPushConstant(uint32_t size, VkShaderStageFlags stages);

        void Assemble();
    };
}