//
// Created by piero on 3/08/2024
//
module;
#include <iostream>
#include <vector>
#define VOLK_IMPLEMENTATION
#include <volk.h>

export module Lettuce:TextureView;

import :Device;
import :Utils;
import :Texture;

export namespace Lettuce::Core
{
    class TextureView
    {
    public:
        Device _device;
        Texture _texture;
        VkImageView _imageView;

        void Build(Device &device, Texture &texture)
        {
            _device = device;
            _texture = texture;

            VkImageViewCreateInfo imageViewCI = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = texture._image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                // and arrays
                .format = VK_FORMAT_B8G8R8A8_SRGB,
                .components =
                    {
                        .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                    },
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = texture._mipLevels,
                    .baseArrayLayer = 0,
                    .layerCount = texture._layerCount,
                },
            };
            checkResult(vkCreateImageView(_device._device, &imageViewCI, nullptr, &_imageView), "image view created sucessfully");
        }

        void Destroy()
        {
            vkDestroyImageView(_device._device, _imageView, nullptr);
        }
    };
}