//
// Created by piero on 26/06/2024
//
module;
#include <iostream>
#include <vector>
#define VOLK_IMPLEMENTATION
#include <volk.h>

export module Lettuce:Descriptor;

import :Device;
import :Utils;

export namespace Lettuce::Core
{
    class Descriptor
    {
        Device _device;
        VkDescriptorPool _descriptorPool;
        VkDescriptorSet _descriptorSet;

        void Create(Device &device)
        {
            _device = device;

            // VkDescriptorPoolSize size = {
            //     .type = VkDescriptorType::
            // }

            // VkDescriptorPoolCreateInfo descriptorPoolCI = {

            // };
        }

        void Destroy()
        {
        }
    };
}