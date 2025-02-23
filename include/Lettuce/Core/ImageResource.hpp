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
    class ImageResource : public IResource
    {
    public:
        std::shared_ptr<Device> _device;
        VkImage _image;
        VkImageLayout _layout;
        uint32_t _mipLevels, _layerCount;
        VkFormat _format;

        ImageResource(const std::shared_ptr<Device> &device, uint32_t width, uint32_t height, uint32_t depth,
                    VkImageType type,
                    VkImageUsageFlags imageUsage,
                    uint32_t mipLevels,
                    uint32_t layerCount,
                    VkFormat format,
                    VkImageLayout initialLayout);
                    
        VkFormat GetFormat();
        ResourceType GetResourceType();
        ResourceLinearity GetResourceLinearity();
        VkMemoryRequirements GetResourceMemoryRequirements();
    };
}