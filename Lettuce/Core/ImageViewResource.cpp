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

void ImageViewResource::Build(Device &device, std::shared_ptr<ImageResource> resource, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D)
{
    _device = device;
    _image = resource;
    _viewType = viewType;
    _subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = texture->_mipLevels,
        .baseArrayLayer = 0,
        .layerCount = texture->_layerCount,
    };

    VkImageViewCreateInfo imageViewCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = resource->_image,
        .viewType = viewType,
        // and arrays
        .format = texture->GetFormat(),
        .components =
            {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
        .subresourceRange = _subresourceRange,
    };
    checkResult(vkCreateImageView(_device._device, &imageViewCI, nullptr, &_imageView));
}

void ImageViewResource::Destroy()
{
    vkDestroyImageView(_device._device, _imageView, nullptr);
}