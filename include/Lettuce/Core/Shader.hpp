//
// Created by piero on 6/03/2025.
//
#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "Device.hpp"
#include "Utils.hpp"

namespace Lettuce::Core
{
    class Shader
    {
    public:
        VkShaderEXT _executionSet;
        std::shared_ptr<Device> _device;

        Shader(const std::shared_ptr<Device>& device) : _device(device){}
        void Assemble(const VkShaderStageFlagBits& stage, const std::shared_ptr<Descriptors>& descriptors);
    };
}