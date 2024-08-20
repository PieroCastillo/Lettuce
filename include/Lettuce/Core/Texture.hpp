//
// Created by piero on 26/06/2024
//
#pragma once
#include <iostream>
#include <vector>
#include <volk.h>
#include <vma/vk_mem_alloc.h>
#include "Device.hpp"
#include "Utils.hpp"

namespace Lettuce::Core
{
    // TODO: Add Load images capability
    class Texture
    {
    public:
        Device _device;
        VkImage _image;
        VmaAllocation _allocation;
        uint32_t _width, _height, _depth, _mipLevels, _layerCount;
        VkImageLayout imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_PREINITIALIZED;

        void Build(Device &device, uint32_t width, uint32_t height, uint32_t depth = 1,
                   VkImageUsageFlagBits imageUsage = VK_IMAGE_USAGE_SAMPLED_BIT,
                   uint32_t mipLevels = 1,
                   uint32_t layerCount = 1);

        void Destroy();
    };
}