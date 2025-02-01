//
// Created by piero on 29/12/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "Device.hpp"
#include "ImageResource.hpp"

namespace Lettuce::Core
{
    class ImageViewResource
    {
    public:
        std::shared_ptr<Device> _device;
        std::shared_ptr<ImageResource> _image;
        VkImageView _imageView;
        VkImageViewType _viewType;
        VkImageSubresourceRange _subresourceRange;

        void Build(const std::shared_ptr<Device> &device, const std::shared_ptr<ImageResource> &image, std::shared_ptr<ImageResource> resource, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D);

        void Destroy();
    };
}