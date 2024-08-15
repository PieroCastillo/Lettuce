//
// Created by piero on 26/06/2024
//
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <volk.h>

namespace Lettuce::Core
{
    class Descriptor
    {
    public:
        Device _device;
        VkDescriptorSet _descriptorSet;
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        std::vector<VkWriteDescriptorSet> writes;

        void AddUpdateInfo(uint32_t binding, DescriptorType type, Sampler sampler, TextureView view)
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

        template <typename TBufferDataType>
        void AddUpdateInfo(uint32_t binding, Buffer &buffer)
        {
            VkDescriptorBufferInfo descriptorBufferI = {
                .buffer = buffer._buffer,
                .offset = 0,
                .range = sizeof(TBufferDataType),
            };

            VkWriteDescriptorSet write = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = _descriptorSet,
                .dstBinding = binding,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = (VkDescriptorType)bindings[binding].descriptorType,
                .pBufferInfo = &descriptorBufferI,
            };
            writes.push_back(write);
        }

        void Update()
        {
            vkUpdateDescriptorSets(_device._device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
        }
    };
}