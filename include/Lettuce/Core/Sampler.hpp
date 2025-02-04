//
// Created by piero on 3/08/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "Utils.hpp"

namespace Lettuce::Core
{
    class Sampler
    {
    public:
        std::shared_ptr<Device> _device;
        VkSampler _sampler;

        void Build(const std::shared_ptr<Device> &device,
                   SamplerAddressMode addressModeU = SamplerAddressMode::Repeat,
                   SamplerAddressMode addressModeV = SamplerAddressMode::Repeat,
                   SamplerAddressMode addressModeW = SamplerAddressMode::Repeat);

        void Destroy();
    };
}