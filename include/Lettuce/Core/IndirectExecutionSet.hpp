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
#include "PipelineLayout.hpp"
#include "Descriptors.hpp"

namespace Lettuce::Core
{
    class IndirectExecutionSet
    {
    public:
        VkIndirectExecutionSetEXT _executionSet;
        std::shared_ptr<Device> _device;
        std::shared_ptr<PipelineLayout> _pipelineLayout;

        IndirectExecutionSet(const std::shared_ptr<Device> &device, const std::shared_ptr<PipelineLayout> &pipelineLayout)
            : _device(device), _pipelineLayout(pipelineLayout) {}
        void Assemble(const std::vector<std::shared_ptr<Shader>> &initialShaders, uint32_t maxShaderCount = 16);
        void Update(const std::vector<std::shared_ptr<Shader>> &shaders);
        void Release();
    };
}