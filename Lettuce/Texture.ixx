//
// Created by piero on 26/06/2024
//
module;
#include <iostream>
#include <vector>
#define VOLK_IMPLEMENTATION
#include <volk.h>
#include <vma/vk_mem_alloc.h>

export module Lettuce:Texture;

import :Device;
import :Utils;
import :Buffer;

export namespace Lettuce::Core
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
                   uint32_t layerCount = 1)
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

        void Destroy()
        {
            vmaDestroyImage(_device.allocator, _image, _allocation);
        }
    };
}