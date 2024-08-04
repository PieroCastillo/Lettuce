//
// Created by piero on 26/06/2024
//
module;
#include <iostream>
#include <vector>
#define VOLK_IMPLEMENTATION
#include <volk.h>

export module Lettuce:Texture;

import :Device;
import :Utils;

export namespace Lettuce::Core
{
    class TextureView
    {
        Device _device;

        void Build(Device &device)
        {
            _device = device;
        }

        void Destroy()
        {
        }
    };
}