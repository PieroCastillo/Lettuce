//
// Created by piero on 26/06/2024
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <vector>
#include "Lettuce/Core/Texture.hpp"
#include "Lettuce/Core/Device.hpp"

using namespace Lettuce::Core;

Texture(const std::shared_ptr<Device> &device, uint32_t width, uint32_t height, uint32_t depth,
                    VkImageType type,
                    VkImageUsageFlags imageUsage,
                    uint32_t mipLevels,
                    uint32_t layerCount,
                    VkFormat format,
                    VmaMemoryUsage memoryUsage,
                    VkImageLayout initialLayout)
{
    _device = device;
    _width = width;
    _height = height;
    _depth = depth;
    _mipLevels = mipLevels;
    _layerCount = layerCount;
    _type = type;
    _imageFormat = format;
    _imageLayout = initialLayout;

    VkImageCreateInfo imageCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = type,
        .format = format,
        .extent = {width, height, depth},
        .mipLevels = mipLevels,
        .arrayLayers = layerCount,
        .samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
        .tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL,
        .usage = (VkImageUsageFlags)imageUsage,
        .sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = initialLayout,
    };

    VmaAllocationCreateInfo allocationCI = {
        .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = memoryUsage,
        .priority = 1.0f,
    };

    checkResult(vmaCreateImage(_device->allocator, &imageCI, &allocationCI, &_image, &_allocation, nullptr));
}

~Texture::Destroy()
{
    vmaDestroyImage(_device->allocator, _image, _allocation);
}