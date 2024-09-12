//
// Created by piero on 26/06/2024
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Buffer.hpp"
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/Sampler.hpp"
#include "Lettuce/Core/TextureView.hpp"
#include "Lettuce/Core/Descriptor.hpp"

using namespace Lettuce::Core;

void Descriptor::AddUpdateInfo(uint32_t binding, DescriptorType type, Sampler sampler, TextureView view)
{
    VkDescriptorImageInfo descriptorImageInfo = {
        .sampler = sampler._sampler,
        .imageView = view._imageView,
        .imageLayout = view._texture.imageLayout,
    };
    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = _descriptorSet,
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = (VkDescriptorType)bindings[binding].descriptorType,
        .pImageInfo = &descriptorImageInfo,
    };
    writes.push_back(write);
}

void Descriptor::Update()
{
    vkUpdateDescriptorSets(_device._device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
    writes.clear();
}