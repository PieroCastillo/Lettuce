//
// Created by piero on 4/08/2024.
//
module;
#include <iostream>
#define VOLK_IMPLEMENTATION
#include <volk.h>
#include <vector>
#include <string>

export module Lettuce:DescriptorPool;

import :Device;
import :Utils;

export namespace Lettuce::Core
{
    class Sampler
    {
        Device _device;

        void Build(Device &device,)
        {
            _device = device;
        }

        void Destroy()
        {
            
        }
    };
}