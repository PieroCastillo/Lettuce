//
// Created by piero on 3/08/2024.
//
module;
#include <iostream>
#include <vector>
#include <string>
#include "volk.h"

export module Lettuce:Sampler;

import :Device;
import :Utils;

export namespace Lettuce::Core
{
    class Sampler
    {
    public:
        Device _device;
        VkSampler _sampler;

        void Build(Device &device,
                   SamplerAddressMode addressModeU = SamplerAddressMode::Repeat,
                   SamplerAddressMode addressModeV = SamplerAddressMode::Repeat,
                   SamplerAddressMode addressModeW = SamplerAddressMode::Repeat)
        {
            _device = device;

            VkSamplerCreateInfo samplerCI = {
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = VkFilter::VK_FILTER_LINEAR,
                .minFilter = VkFilter::VK_FILTER_LINEAR,
                .mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU = (VkSamplerAddressMode)addressModeU,
                .addressModeV = (VkSamplerAddressMode)addressModeV,
                .addressModeW = (VkSamplerAddressMode)addressModeW,
                .anisotropyEnable = VK_TRUE,
                .maxAnisotropy = 16,
                .maxLod = VK_LOD_CLAMP_NONE,
                .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                .unnormalizedCoordinates = VK_FALSE,
            };

            checkResult(vkCreateSampler(_device._device, &samplerCI, nullptr, &_sampler), "sampler created successfully");
        }

        void Destroy()
        {
            vkDestroySampler(_device._device, _sampler, nullptr);
        }
    };
}