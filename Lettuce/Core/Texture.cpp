//
// Created by piero on 26/06/2024
//
#include <iostream>
#include <vector>
#include <volk.h>
#include <vma/vk_mem_alloc.h>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Texture.hpp"

using namespace Lettuce::Core;

void Texture::Build(Device &device, uint32_t width, uint32_t height, uint32_t depth,
                    VkImageUsageFlagBits imageUsage,
                    uint32_t mipLevels,
                    uint32_t layerCount)
{
    _device = device;
    _width = width;
    _height = height;
    _depth = depth;
    _mipLevels = mipLevels;
    _layerCount = layerCount;

    auto usage = imageUsage | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VkImageCreateInfo imageCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VkImageType::VK_IMAGE_TYPE_2D,
        .format = VkFormat::VK_FORMAT_B8G8R8A8_SRGB,
        .extent = {width, height, depth},
        .mipLevels = mipLevels,
        .arrayLayers = layerCount,
        .samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
        .tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL,
        .usage = (VkImageUsageFlags)usage,
        .sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_PREINITIALIZED,
    };

    VmaAllocationCreateInfo allocationCI = {
        .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        .priority = 1.0f,
    };

    checkResult(vmaCreateImage(_device.allocator, &imageCI, &allocationCI, &_image, &_allocation, nullptr), "image created successfully");
}

void Texture::Destroy()
{
    vmaDestroyImage(_device.allocator, _image, _allocation);
}