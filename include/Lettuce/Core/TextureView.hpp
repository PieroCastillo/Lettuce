//
// Created by piero on 3/08/2024
//
#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Texture.hpp"
#include "Lettuce/Core/TextureView.hpp"

namespace Lettuce::Core
{
    class TextureView
    {
    public:
        std::shared_ptr<Device> _device;
        std::shared_ptr<Texture> _texture;
        VkImageView _imageView;
        VkImageViewType _viewType;
        VkImageSubresourceRange _subresourceRange;

        void Build(const std::shared_ptr<Device> &device, std::shared_ptr<Texture> texture, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D);
        
        void Destroy();
    };
}