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
        std::shared_ptr<Device> _device;
        VkImage _image;
        void Create(const std::shared_ptr<Device>& device);
        ResourceType GetResourceType();
        ResourceLinearity GetResourceLinearity();
        VkMemoryRequirements GetResourceMemoryRequirements();
        virtual std::shared_ptr<IResource> GetReference();
    };
}