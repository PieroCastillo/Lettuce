//
// Created by piero on 26/06/2024
//
module;
#include <iostream>
#include <vector>
#include <map>
#include <string>
#define VOLK_IMPLEMENTATION
#include <volk.h>

export module Lettuce:Descriptor;

import :Device;
import :Utils;
import :Buffer;

export namespace Lettuce::Core
{
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

        void BuildLayout(Device &device)
        {
            _device = device;

            VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .bindingCount = (uint32_t)bindings.size(),
                .pBindings = bindings.data(),
            };
            checkResult(vkCreateDescriptorSetLayout(_device._device, &descriptorSetLayoutCI, nullptr, &_setLayout), "Descriptor set Layout created successfully");
        }

        void Build()
        {
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

            for (auto size : sizes)
            {
                std::cout << "descriptor count: " << size.descriptorCount << "descriptor type int: " << size.type << std::endl;
            }

            VkDescriptorPoolCreateInfo descriptorPoolCI = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
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
        void Update(uint32_t binding, Buffer &buffer)
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
                .descriptorType = bindingsMap[binding],
                .pBufferInfo = &descriptorBufferI,
            };
            vkUpdateDescriptorSets(_device._device, 1, &write, 0, nullptr);
        }

        template <typename TBufferDataType>
        void Update(uint32_t binding, std::vector<Buffer> &buffers)
        {
            std::vector<VkDescriptorBufferInfo> buffersI;
            for (auto &buffer : buffers)
            {
                VkDescriptorBufferInfo descriptorBufferI = {
                    .buffer = buffer._buffer,
                    .offset = 0,
                    .range = sizeof(TBufferDataType),
                };
                std::cout << "buffer ptr in Descriptor: " << buffer._buffer << std::endl;
                std::cout << "object size in Descriptor: " << std::to_string(sizeof(TBufferDataType)) << std::endl;
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