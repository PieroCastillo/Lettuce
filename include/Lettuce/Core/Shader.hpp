//
// Created by piero on 6/03/2025.
//
#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "Device.hpp"
#include "Descriptors.hpp"
#include "PipelineLayout.hpp"
#include "Utils.hpp"

namespace Lettuce::Core
{
    class Shader
    {
    private:
        std::vector<VkSpecializationMapEntry> entries;
        std::vector<void*> datas; 
        uint32_t currentOffset = 0;

    public:
        VkShaderEXT _shader;
        std::shared_ptr<Device> _device;
        std::shared_ptr<PipelineLayout> _layout;
        std::shared_ptr<Descriptors> _descriptors;

        Shader(const std::shared_ptr<Device> &device, const std::shared_ptr<PipelineLayout> &layout, const std::shared_ptr<Descriptors> &descriptors) : _device(device), _layout(layout), _descriptors(descriptors)
        {
        }

        void Assemble(const VkShaderStageFlagBits &stage, const std::vector<uint32_t> &code);

        void AddConstant(uint32_t constantId, uint32_t size, void *data);
    };
}