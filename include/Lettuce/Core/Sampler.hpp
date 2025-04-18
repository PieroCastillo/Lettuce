//
// Created by piero on 3/08/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "Utils.hpp"
#include "IReleasable.hpp"
#include "IManageHandle.hpp"

namespace Lettuce::Core
{
    class Sampler : public IReleasable, public IManageHandle<VkSampler>
    {
    public:
        std::shared_ptr<Device> _device;

        Sampler(const std::shared_ptr<Device> &device,
                   SamplerAddressMode addressModeU = SamplerAddressMode::Repeat,
                   SamplerAddressMode addressModeV = SamplerAddressMode::Repeat,
                   SamplerAddressMode addressModeW = SamplerAddressMode::Repeat);

        void Release();
    };
}