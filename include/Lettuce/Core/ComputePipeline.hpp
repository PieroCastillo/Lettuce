//
// Created by piero on 14/02/2024.
//
#pragma once
#include <iostream>
#include <memory>
#include "Device.hpp"
#include "IReleasable.hpp"
#include "PipelineLayout.hpp"
#include "ShaderModule.hpp"
#include "IManageHandle.hpp"

namespace Lettuce::Core
{
    class ComputePipeline : public IReleasable, public IManageHandle<VkPipeline>
    {
    public:
        std::shared_ptr<Device> _device;
        std::shared_ptr<PipelineLayout> _pipelineLayout;

        ComputePipeline(const std::shared_ptr<Device> &device, const std::shared_ptr<PipelineLayout> &connector, const std::shared_ptr<ShaderModule> &shader);

        void Release();
    };
}