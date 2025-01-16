//
// Created by piero on 22/12/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "Device.hpp"
#include "Utils.hpp"

namespace Lettuce::Core
{
    class IResource
    {
    public:
        uint32_t offset;
        virtual ResourceType GetResourceType();
        virtual ResourceLinearity GetResourceLinearity();
        virtual VkMemoryRequirements GetResourceMemoryRequirements();
        virtual std::shared_ptr<IResource> GetReference();
    };
}