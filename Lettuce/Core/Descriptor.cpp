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

void Descriptor::Build(Device &device, uint32_t maxSets)
{
    _device = device;
    // create descriptor set layouts
    int index = 0;
    for (std::pair<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> bindings : bindingsMap)
    {
        // for each binding increments +1 per type
        for (auto binding : bindings.second)
        {
            typesMap[binding.descriptorType] += 1;
        }

        const VkDescriptorBindingFlags flag =
            VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
            VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;

        std::vector<VkDescriptorBindingFlags> flags(bindings.second.size(), flag);
        VkDescriptorSetLayoutCreateInfo layoutCI = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &flags,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            .bindingCount = (uint32_t)bindings.second.size(),
            .pBindings = bindings.second.data(),
        };
        checkResult(vkCreateDescriptorSetLayout(device._device, &layoutCI, nullptr, &(_layouts[index])));
        index++;
    }

    // creates the sizes required for the Descriptor Pool
    for (auto typePair : typesMap)
    {
        sizes.push_back(VkDescriptorPoolSize{
            .type = typePair.first,
            .descriptorCount = typePair.second,
        });
    }

    // creates pool
    VkDescriptorPoolCreateInfo poolCI = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        .poolSizeCount = (uint32_t)sizes.size(),
        .pPoolSizes = sizes.data(),
    };
    checkResult(vkCreateDescriptorPool(device._device, &poolCI, nullptr, &_pool));

    // creates descriptor set
    VkDescriptorSetAllocateInfo descriptorSetAI = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = _pool,
        .descriptorSetCount = 1,
        .pSetLayouts = _layouts.data(),
    };
    checkResult(vkAllocateDescriptorSets(device._device, &descriptorSetAI, &_descriptorSet));
}

void Descriptor::Destroy()
{
    vkFreeDescriptorSets(_device._device, _pool, 1, &_descriptorSet);
    vkDestroyDescriptorPool(_device._device, _pool, nullptr);
    for (auto layout : _layouts)
    {
        vkDestroyDescriptorSetLayout(_device._device, layout, nullptr);
    }
    bindingsMap.clear();
    _layouts.clear();
    typesMap.clear();
    sizes.clear();
}

void Descriptor::AddBinding(uint32_t set, uint32_t binding, DescriptorType type, PipelineStage stage, uint32_t maxBindings)
{
    bindingsMap[set].push_back(VkDescriptorSetLayoutBinding{
        .binding = binding,
        .descriptorType = (VkDescriptorType)type,
        .descriptorCount = maxBindings,
        .stageFlags = (VkShaderStageFlags)stage,
    });
}

void Descriptor::AddUpdateInfo(uint32_t set, uint32_t binding, std::vector<TextureView> views)
{

}

void Descriptor::AddUpdateInfo(uint32_t set, uint32_t binding, std::vector<Sampler> samplers)
{
    std::vector<VkDescriptorImageInfo> imageInfos(samplers.size());
    for (int i = 0; i < samplers.size(); i++)
    {
        imageInfos.push_back(VkDescriptorImageInfo{
            .sampler = samplers[i]._sampler,
        });
    }
    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = set,
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .pImageInfo = imageInfos.data(),
    };
    writes.push_back(write);
}

void Descriptor::AddUpdateInfo(uint32_t set, uint32_t binding, std::vector<Sampler> samplers, std::vector<TextureView> views)
{
    if (samplers.size() != views.size())
    {
        throw std::runtime_error("views and samplers should have the same size");
    }
    std::vector<VkDescriptorImageInfo> imageInfos(samplers.size());
    for (int i = 0; i < samplers.size(); i++)
    {
        imageInfos.push_back(VkDescriptorImageInfo{
            .sampler = samplers[i]._sampler,
            .imageView = views[i]._imageView,
            .imageLayout = views[i]._texture.imageLayout,
        });
    }
    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
       // .dstSet = set,
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = imageInfos.data(),
    };
    writes.push_back(write);
}

void Descriptor::Update()
{
    vkUpdateDescriptorSets(_device._device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
    writes.clear();
}