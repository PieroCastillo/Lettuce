//
// Created by piero on 6/03/2025.
//
#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "Device.hpp"
#include "IReleasable.hpp"
#include "Utils.hpp"
#include "Shader.hpp"
#include "PipelineLayout.hpp"
#include "Descriptors.hpp"
#include "IndirectCommandsLayout.hpp"

namespace Lettuce::Core
{
    class IndirectExecutionSet : public IReleasable
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
        VkMemoryRequirements GetRequirements(const std::shared_ptr<IndirectCommandsLayout>& layout, uint32_t maxSequenceCount, uint32_t maxDrawCount);
    };
}