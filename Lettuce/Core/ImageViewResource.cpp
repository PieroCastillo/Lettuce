//
// Created by piero on 3/08/2024
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/ImageResource.hpp"
#include "Lettuce/Core/ImageViewResource.hpp"

using namespace Lettuce::Core;

void ImageViewResource::Build(const std::shared_ptr<Device> &device, const std::shared_ptr<ImageResource> &image, std::shared_ptr<ImageResource> resource, VkImageViewType viewType)
{
    _device = device;
    _image = image;
    _viewType = viewType;
    _subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = image->_mipLevels,
        .baseArrayLayer = 0,
        .layerCount = image->_layerCount,
    };

    VkImageViewCreateInfo imageViewCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = resource->_image,
        .viewType = viewType,
        // and arrays
        .format = image->_format,
        .components =
            {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
        .subresourceRange = _subresourceRange,
    };
    checkResult(vkCreateImageView(_device->_device, &imageViewCI, nullptr, &_imageView));
}

void ImageViewResource::Destroy()
{
    vkDestroyImageView(_device->_device, _imageView, nullptr);
}