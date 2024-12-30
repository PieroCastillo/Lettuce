//
// Created by piero on 29/12/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include "IResource.hpp"
#include "Device.hpp"

namespace Lettuce::Core
{
    class BufferResource : public IRecordInfo
    {
    public:
        VkBuffer _buffer;
        void Create(Device &device);
        ResourceType GetResourceType();
        ResourceLinearity GetResourceLinearity();
        VkMemoryRequirements GetResourceMemoryRequirements();
        void *GetReference();
    };
}