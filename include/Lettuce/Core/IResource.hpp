//
// Created by piero on 22/12/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "Device.hpp"
#include "IReleasable.hpp"
#include "Utils.hpp"

namespace Lettuce::Core
{
    class IResource
    {
    public:
        uint32_t offset;
        virtual ResourceType GetResourceType() = 0;
        virtual ResourceLinearity GetResourceLinearity() = 0;
        virtual VkMemoryRequirements GetResourceMemoryRequirements() = 0;
    };
}