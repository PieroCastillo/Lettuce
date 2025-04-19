//
// Created by piero on 18/04/2025.
//
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
#include <memory>
#include "Lettuce/Core/Common.hpp"
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/DescriptorLayoutGroup.hpp"
#include "Lettuce/Core/Utils.hpp"

using namespace Lettuce::Core;

void DescriptorLayoutGroup::AddBinding(uint32_t set,
                                       uint32_t binding,
                                       VkDescriptorType type,
                                       VkShaderStageFlags stages,
                                       uint32_t descriptorCount)
{
    bindings[set].push_back({
        .binding = binding,
        .descriptorType = type,
        .descriptorCount = descriptorCount,
        .stageFlags =  stages,
    });
}

void DescriptorLayoutGroup::Assemble()
{
    for (auto &[setIdx, bindingsVec] : bindings)
    {
        VkDescriptorSetLayoutCreateInfo layoutCI = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = (uint32_t)bindingsVec.size(),
            .pBindings = bindingsVec.data(),
        };

        if (!allowDescriptorBuffers)
        {
            const VkDescriptorBindingFlags flag =
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
                VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;

            std::vector<VkDescriptorBindingFlags> flags(bindingsVec.size(), flag);

            VkDescriptorSetLayoutBindingFlagsCreateInfo layoutBindingFlagsCI = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
                .bindingCount = (uint32_t)flags.size(),
                .pBindingFlags = flags.data(),
            };

            layoutCI.pNext = &layoutBindingFlagsCI;
            layoutCI.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        }
        else
        {
            layoutCI.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
        }

        VkDescriptorSetLayout layout;
        checkResult(vkCreateDescriptorSetLayout(device->GetHandle(), &layoutCI, nullptr, &layout));
        AddHandle(layout);
    }
}

void DescriptorLayoutGroup::Release()
{
    for (auto &layout : GetHandles())
    {
        vkDestroyDescriptorSetLayout(device->GetHandle(), layout, nullptr);
    }
}