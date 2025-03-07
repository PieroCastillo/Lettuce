//
// Created by piero on 6/03/2025.
//
#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "Device.hpp"
#include "Utils.hpp"
#include "Shader.hpp"

namespace Lettuce::Core
{
    class IndirectExecutionSet
    {
    public:
        VkIndirectExecutionSetEXT _executionSet;
        std::shared_ptr<Device> _device;

        IndirectExecutionSet(const std::shared_ptr<Device>& device) : _device(device){}
        void Assemble();
        void Update(const std::vector<std::shared_ptr<Shader>>& shaders);
        void Release();
    };
}