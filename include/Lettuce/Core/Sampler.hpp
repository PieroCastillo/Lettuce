//
// Created by piero on 3/08/2024.
//
#pragma once
#include <iostream>
#include <volk.h>
#include <vector>
#include <string>
#include "Utils.hpp"

namespace Lettuce::Core
{
    class Sampler
    {
    public:
        Device _device;
        VkSampler _sampler;

        void Build(Device &device,
                   SamplerAddressMode addressModeU = SamplerAddressMode::Repeat,
                   SamplerAddressMode addressModeV = SamplerAddressMode::Repeat,
                   SamplerAddressMode addressModeW = SamplerAddressMode::Repeat);

        void Destroy();
    };
}