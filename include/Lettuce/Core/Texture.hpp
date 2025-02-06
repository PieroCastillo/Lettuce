//
// Created by piero on 26/06/2024
//
#pragma once
#include <iostream>
#include <vector>
#include "Device.hpp"
#include "Utils.hpp"

namespace Lettuce::Core
{
    // TODO: Add Load images capability
    class Texture
    {
    private:

    public:
        std::shared_ptr<Device> _device;
        VkImage _image;
        VmaAllocation _allocation;
        uint32_t _width, _height, _depth, _mipLevels, _layerCount;
        VkFormat _imageFormat;
        VkImageType _type;
        VkImageLayout _imageLayout;

        Texture(const std::shared_ptr<Device> &device, uint32_t width, uint32_t height, uint32_t depth = 1,
                   VkImageType type = VK_IMAGE_TYPE_2D,
                   VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_SAMPLED_BIT,
                   uint32_t mipLevels = 1,
                   uint32_t layerCount = 1,
                   VkFormat format = VkFormat::VK_FORMAT_B8G8R8A8_SRGB,
                   VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                   VkImageLayout initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED);

        ~Texture();
    };
}