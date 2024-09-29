//
// Created by piero on 3/08/2024
//
#pragma once
#include <iostream>
#include <vector>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Texture.hpp"
#include "Lettuce/Core/TextureView.hpp"

namespace Lettuce::Core
{
    class TextureView
    {
    public:
        Device _device;
        Texture _texture;
        VkImageView _imageView;
        VkImageViewType _viewType;

        void Build(Device &device, Texture &texture, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D);
        
        void Destroy();
    };
}