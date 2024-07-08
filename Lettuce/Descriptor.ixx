//
// Created by piero on 26/06/2024
//
module;
#include <iostream>
#include <vector>
#include <map>
#define VOLK_IMPLEMENTATION
#include <volk.h>

export module Lettuce:Descriptor;

import :Device;
import :Utils;
import :Buffer;

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
    };

    class Descriptor
    {
    public:
        Device _device;
        VkDescriptorSetLayout _setLayout;
        VkDescriptorPool _descriptorPool;
        VkDescriptorSet _descriptorSet;

        std::vector<VkDescriptorSetLayoutBinding> bindings;
        std::map<uint32_t, VkDescriptorType> bindingsMap;
        std::map<VkDescriptorType, uint32_t> typesMap;

        void AddDescriptorBinding(uint32_t binding,
                                  DescriptorType type,
                                  uint32_t descriptorCount,
                                  PipelineStage stage)
        // const VkSampler *pImmutableSamplers)
        {
            VkDescriptorSetLayoutBinding setBinding = {
                .binding = binding,
                .descriptorType = (VkDescriptorType)type,
                .descriptorCount = 1,
                .stageFlags = (VkShaderStageFlags)stage,
                .pImmutableSamplers = nullptr,
            };
            typesMap[(VkDescriptorType)type]++;
            bindingsMap[binding] = ((VkDescriptorType)type);
            bindings.emplace_back(setBinding);
        }

        void Build(Device &device)
        {
            _device = device;

            VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .bindingCount = (uint32_t)bindings.size(),
                .pBindings = bindings.data(),
            };
            checkResult(vkCreateDescriptorSetLayout(_device._device, &descriptorSetLayoutCI, nullptr, &_setLayout), "Descriptor set Layout created successfully");

            std::vector<VkDescriptorPoolSize> sizes;
            uint32_t maxSets = 0;
            for (auto t : typesMap)
            {
                VkDescriptorPoolSize size = {
                    .type = t.first,
                    .descriptorCount = t.second,
                };
                maxSets += t.second;
                sizes.emplace_back(size);
            }

            VkDescriptorPoolCreateInfo descriptorPoolCI = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                // VkDescriptorPoolCreateFlags flags;
                .maxSets = 1,
                .poolSizeCount = (uint32_t)sizes.size(),
                .pPoolSizes = sizes.data(),
            };
            checkResult(vkCreateDescriptorPool(_device._device, &descriptorPoolCI, nullptr, &_descriptorPool), "DescriptorPool created sucessfully");

            VkDescriptorSetAllocateInfo descriptorSetAI = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = _descriptorPool,
                .descriptorSetCount = 1,
                .pSetLayouts = &_setLayout,
            };
            checkResult(vkAllocateDescriptorSets(_device._device, &descriptorSetAI, &_descriptorSet), "DescriptorSet allocated sucessfully");
        };

        template <typename TBufferDataType>
        void Update(uint32_t binding, std::vector<Buffer> &buffers)
        {
            std::vector<VkDescriptorBufferInfo> buffersI;
            for (auto buffer : buffers)
            {
                VkDescriptorBufferInfo descriptorBufferI = {
                    .buffer = buffer._buffer,
                    .offset = 0,
                    .range = sizeof(TBufferDataType),
                };
                buffersI.emplace_back(descriptorBufferI);
            }

            VkWriteDescriptorSet write = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = _descriptorSet,
                .dstBinding = binding,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = bindingsMap[binding],
                .pBufferInfo = buffersI.data(),
            };
            vkUpdateDescriptorSets(_device._device, 1, &write, 0, nullptr);
        }

        void Destroy()
        {
            vkFreeDescriptorSets(_device._device, _descriptorPool, 1, &_descriptorSet);
            vkDestroyDescriptorPool(_device._device, _descriptorPool, nullptr);
            vkDestroyDescriptorSetLayout(_device._device, _setLayout, nullptr);
        }
    };
}