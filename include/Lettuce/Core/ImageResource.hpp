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
    class ImageResource : public IRecordInfo
    {
    public:
        VkImage _image;
        void Create(Device &device);
        ResourceType GetResourceType();
        ResourceLinearity GetResourceLinearity();
        uint32_t GetSize();
        uint32_t GetAlignment();
        VkMemoryRequirements GetResourceMemoryRequirements();
        void *GetReference();
    };
}