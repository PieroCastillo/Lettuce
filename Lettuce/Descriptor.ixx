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
    enum class DescriptorType
    {
        Sampler = 0,
        CombinedImageSampler = 1,
        SampledImage = 2,
        StorageImage = 3,
        UniformTexelBuffer = 4,
        StorageTexelBuffer = 5,
        UniformBuffer = 6,
        StorageBuffer = 7,
        UniformBufferDynamic = 8,
        StorageBufferDynamic = 9,
        InputAttachment = 10,
        // Provided by VK_VERSION_1_3
        InlineUniformBlock = 1000138000,
        // Provided by VK_KHR_acceleration_structure
        AccelerationStructure = 1000150000,
        Mutable = 1000351000,
    }

    class Descriptor
    {
    public:
        Device _device;
        VkDescriptorSetLayout _setLayout;
        VkDescriptorPool _descriptorPool;
        VkDescriptorSet _descriptorSet;

        std::vector<VkDescriptorSetLayoutBinding> bindings;

        void AddDescriptorBinding(uint32_t binding,
                                  DescriptorType type,
                                  uint32_t descriptorCount,
                                  PipelineStage stage)
        // const VkSampler *pImmutableSamplers)
        {
            VkDescriptorSetLayoutBinding binding = {
                .binding = binding,
                .descriptorType = (VkDescriptorType)type,
                .descriptorCount = 1,
                .stageFlags = (VkShaderStageFlags)stage,
                .pImmutableSamplers = nullptr,
            };
            bindings.emplace_back(binding);
        }

        void BuildLayout(Device &device)
        {
            _device = device;

            VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .bindingCount = (uint32_t)bindings.size(), 
                .pBindings = bindings.data(),
            };
            checkResult(vkCreateDescriptorSetLayout(_device._device, &descriptorSetLayoutCI, nullptr, &_setLayout), "Descriptor set Layout created successfully");

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