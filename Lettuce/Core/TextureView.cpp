//
// Created by piero on 3/08/2024
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <vector>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Texture.hpp"
#include "Lettuce/Core/TextureView.hpp"

using namespace Lettuce::Core;

void TextureView::Build(Device &device, std::shared_ptr<Texture> texture, VkImageViewType viewType)
{
    _device = device;
    _texture = texture;
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
        .image = texture->_image,
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
    checkResult(vkCreateImageView(_device._device, &imageViewCI, nullptr, &_imageView), "image view created sucessfully");
}

void TextureView::Destroy()
{
    vkDestroyImageView(_device._device, _imageView, nullptr);
}
