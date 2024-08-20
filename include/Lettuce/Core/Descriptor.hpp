//
// Created by piero on 26/06/2024
//
#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <volk.h>
#include "Device.hpp"
#include "Buffer.hpp"
#include "Sampler.hpp"
#include "Utils.hpp"
#include "TextureView.hpp"

namespace Lettuce::Core
{
    class Descriptor
    {
    public:
        Device _device;
        VkDescriptorSet _descriptorSet;
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        std::vector<VkWriteDescriptorSet> writes;

        void AddUpdateInfo(uint32_t binding, DescriptorType type, Sampler sampler, TextureView view);

        template <typename TBufferDataType>
        void AddUpdateInfo(uint32_t binding, Buffer &buffer);
        
        void Update();
    };
}