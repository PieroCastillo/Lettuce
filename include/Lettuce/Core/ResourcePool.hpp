//
// Created by piero on 22/12/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include "IResource.hpp"
#include "Device.hpp"

namespace Lettuce::Core
{
    class ResourcePool
    {
    public:
        std::vector<std::unique_ptr<IResource>> resourcePtrs;
        VkDeviceMemory _memory;
        void Bind(Device &device);
    };
}