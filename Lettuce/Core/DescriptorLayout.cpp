//
// Created by piero on 4/08/2024.
//
#include <iostream>
#include <volk.h>
#include <vector>
#include <string>
#include <set>
#include <volk.h>
#include "Lettuce/Core/DescriptorLayout.hpp"

using namespace Lettuce::Core;

void DescriptorLayout::AddDescriptorBinding(uint32_t binding,
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

void DescriptorLayout::Build(Device &device, std::vector<uint32_t> descriptorCounts, uint32_t maxDescriptorSets, uint32_t maxDescriptorCount)
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
    BuildInternal(descriptorCounts);
}

void DescriptorLayout::BuildPool()
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

void DescriptorLayout::BuildInternal(std::vector<uint32_t> descriptorCounts)
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

std::vector<Descriptor> DescriptorLayout::GetDescriptors()
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

void DescriptorLayout::Reset()
{
    checkResult(vkResetDescriptorPool(_device._device, _descriptorPool, 0), "DescriptorPool reset successfully");
}

void DescriptorLayout::Destroy()
{
    vkFreeDescriptorSets(_device._device, _descriptorPool, (uint32_t)_descriptorSets.size(), _descriptorSets.data());
    vkDestroyDescriptorPool(_device._device, _descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(_device._device, _setLayout, nullptr);
}