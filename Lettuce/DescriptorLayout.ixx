//
// Created by piero on 4/08/2024.
//
module;
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include "volk.h"

export module Lettuce:DescriptorLayout;

import :Device;
import :Utils;
import :Descriptor;

export namespace Lettuce::Core
{
    class DescriptorLayout
    {
    public:
        Device _device;
        VkDescriptorSetLayout _setLayout;
        VkDescriptorPool _descriptorPool;
        uint32_t maxDescriptorSets;
        uint32_t maxDescriptorCount;
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        std::set<VkDescriptorType> usedTypes;
        std::vector<VkDescriptorSet> _descriptorSets;

        void AddDescriptorBinding(uint32_t binding,
                                  DescriptorType type,
                                  PipelineStage stage)
        // const VkSampler *pImmutableSamplers)
        {
            uint32_t &ref = maxDescriptorCount;
            VkDescriptorSetLayoutBinding setBinding = {
                .binding = binding,
                .descriptorType = (VkDescriptorType)type,
                .descriptorCount = ref,
                .stageFlags = (VkShaderStageFlags)stage,
                .pImmutableSamplers = nullptr,
            };
            usedTypes.insert((VkDescriptorType)type);
            bindings.emplace_back(setBinding);
        }

        void Build(Device &device, std::vector<uint32_t> descriptorCounts, uint32_t maxDescriptorSets = 1, uint32_t maxDescriptorCount = 1)
        {
            _device = device;
            this->maxDescriptorSets = maxDescriptorSets;
            this->maxDescriptorCount = maxDescriptorCount;

            const VkDescriptorBindingFlags flag =
                VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
                VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;

            std::vector<VkDescriptorBindingFlags> flags(bindings.size(), flag);

            VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
                .bindingCount = (uint32_t)flags.size(),
                .pBindingFlags = flags.data(),
            };
            VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = &bindingFlags,
                .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            };

            checkResult(vkCreateDescriptorSetLayout(_device._device, &descriptorSetLayoutCI, nullptr, &_setLayout), "Descriptor Set Layout created successfully");
            BuildPool();
            Build(descriptorCounts);
        }

        void BuildPool()
        {
            std::vector<VkDescriptorPoolSize> sizes(usedTypes.size());

            for (auto type : usedTypes)
            {
                sizes.push_back({type, maxDescriptorSets * maxDescriptorCount});
            }

            VkDescriptorPoolCreateInfo descriptorPoolCI = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .flags = VkDescriptorPoolCreateFlagBits::VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
                .maxSets = maxDescriptorSets,
                .poolSizeCount = (uint32_t)sizes.size(),
                .pPoolSizes = sizes.data(),
            };
            checkResult(vkCreateDescriptorPool(_device._device, &descriptorPoolCI, nullptr, &_descriptorPool), "DescriptorPool created sucessfully");
        }

        void Build(std::vector<uint32_t> descriptorCounts)
        {
            VkDescriptorSetVariableDescriptorCountAllocateInfo variableAI = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
                .descriptorSetCount = (uint32_t)descriptorCounts.size(),
                .pDescriptorCounts = descriptorCounts.data(),
            };

            VkDescriptorSetAllocateInfo descriptorSetAI = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .pNext = &variableAI,
                .descriptorPool = _descriptorPool,
                .descriptorSetCount = (uint32_t)descriptorCounts.size(),
                .pSetLayouts = &_setLayout,
            };
            checkResult(vkAllocateDescriptorSets(_device._device, &descriptorSetAI, _descriptorSets.data()), "DescriptorSet allocated sucessfully");
        };

        std::vector<Descriptor> GetDescriptors()
        {
            std::vector<Descriptor> descriptors(_descriptorSets.size());

            for (int i = 0; i < _descriptorSets.size(); i++)
            {
                Descriptor descriptor;
                descriptor._descriptorSet = _descriptorSets[i];
                descriptor.bindings = this->bindings;
                descriptors[i] = descriptor;
            }

            return descriptors;
        }

        void Reset()
        {
            checkResult(vkResetDescriptorPool(_device._device, _descriptorPool, 0), "DescriptorPool reset successfully");
        }

        void Destroy()
        {
            vkFreeDescriptorSets(_device._device, _descriptorPool, (uint32_t)_descriptorSets.size(), _descriptorSets.data());
            vkDestroyDescriptorPool(_device._device, _descriptorPool, nullptr);
            vkDestroyDescriptorSetLayout(_device._device, _setLayout, nullptr);
        }
    };
}