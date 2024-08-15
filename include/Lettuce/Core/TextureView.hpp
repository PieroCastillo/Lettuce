//
// Created by piero on 3/08/2024
//
#pragma once
#include <iostream>
#include <vector>
#include <volk.h>
#include <Device.hpp>
#include <Texture.hpp>

namespace Lettuce::Core
{
    class TextureView
    {
    public:
        Device _device;
        Texture _texture;
        VkImageView _imageView;

        void Build(Device &device, Texture &texture);
        
        void Destroy();
    };
}