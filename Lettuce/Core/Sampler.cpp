//
// Created by piero on 3/08/2024.
//
#include <iostream>
#include <volk.h>
#include <vector>
#include <string>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Sampler.hpp"

using namespace Lettuce::Core;

void Sampler::Build(Device &device,
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

void Sampler::Destroy()
{
    vkDestroySampler(_device._device, _sampler, nullptr);
}