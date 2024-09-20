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
    _layouts.resize(bindingsMap.size());
    for (auto &[_, bindings] : bindingsMap)
    {
        // for each binding increments +1 per type
        int j = 0;
        for (auto binding : bindings)
        {
            typesMap[binding.descriptorType] += 1;
            j++;
        }

        const VkDescriptorBindingFlags flag =
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
            VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;

        std::vector<VkDescriptorBindingFlags> flags(bindings.size(), flag);
        VkDescriptorSetLayoutBindingFlagsCreateInfo layoutBindingFlagsCI = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .bindingCount = (uint32_t)flags.size(),
            .pBindingFlags = flags.data(),
        };
        VkDescriptorSetLayoutCreateInfo layoutCI = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &layoutBindingFlagsCI,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            .bindingCount = (uint32_t)bindings.size(),
            .pBindings = bindings.data(),
        };
        VkDescriptorSetLayout layout;
        checkResult(vkCreateDescriptorSetLayout(device._device, &layoutCI, nullptr, &layout));
        _layouts[index] = layout;
        index++;
    }

    // creates the sizes required for the Descriptor Pool
    for (auto &[type, count] : typesMap)
    {
        sizes.push_back(VkDescriptorPoolSize{
            .type = type,
            .descriptorCount = count,
        });
    }

    // creates pool
    VkDescriptorPoolCreateInfo poolCI = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        .maxSets = maxSets,
        .poolSizeCount = (uint32_t)sizes.size(),
        .pPoolSizes = sizes.data(),
    };
    checkResult(vkCreateDescriptorPool(device._device, &poolCI, nullptr, &_pool));

    // creates descriptor set
    _descriptorSets.resize(_layouts.size(), VK_NULL_HANDLE);
    VkDescriptorSetAllocateInfo descriptorSetAI = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = _pool,
        .descriptorSetCount = (uint32_t)_descriptorSets.size(),
        .pSetLayouts = _layouts.data(),
    };
    checkResult(vkAllocateDescriptorSets(device._device, &descriptorSetAI, _descriptorSets.data()));
}

void Descriptor::Destroy()
{
    vkFreeDescriptorSets(_device._device, _pool, 1, _descriptorSets.data());
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

void Descriptor::AddBinding(uint32_t set, uint32_t binding, DescriptorType type, PipelineStage stage, uint32_t descriptorCount)
{
    VkDescriptorSetLayoutBinding layoutBinding = {
        .binding = binding,
        .descriptorType = (VkDescriptorType)type,
        .descriptorCount = descriptorCount,
        .stageFlags = (VkShaderStageFlags)stage,
    };
    bindingsMap[set].push_back(layoutBinding);
    bindingsTypes[{set, binding}] = (VkDescriptorType)type;
}

void Descriptor::AddUpdateInfo(uint32_t set, uint32_t binding, uint32_t size, std::vector<Buffer> buffers)
{
    std::vector<VkDescriptorBufferInfo> bufferInfos(buffers.size());
    for (int i = 0; i < buffers.size(); i++)
    {
        VkDescriptorBufferInfo bufferI = {
            .buffer = buffers[i]._buffer,
            .offset = 0,
            .range = size,
        };
        bufferInfos[i] = bufferI;
    }
    // this is needed for disabling the auto-deleting of the bufferInfos vector
    writesFieldsMap[{set, binding}].bufferInfos = bufferInfos;

    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = _descriptorSets[(int)set],
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = (uint32_t)buffers.size(),
        .descriptorType = bindingsTypes[{set, binding}],
        .pBufferInfo = writesFieldsMap[{set, binding}].bufferInfos.data(),
    };
    writes.push_back(write);
}

void Descriptor::AddUpdateInfo(uint32_t set, uint32_t binding, std::vector<TextureView> views)
{
    std::vector<VkDescriptorImageInfo> imageInfos(views.size());
    for (int i = 0; i < views.size(); i++)
    {
        imageInfos[i] = VkDescriptorImageInfo{
            .imageView = views[i]._imageView,
            .imageLayout = views[i]._texture.imageLayout,
        };
    }
    writesFieldsMap[{set, binding}].imageInfos = imageInfos;
    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = _descriptorSets[(int)set],
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = (uint32_t)views.size(),
        .descriptorType = bindingsTypes[{set, binding}],
        .pImageInfo = imageInfos.data(),
    };
    writes.push_back(write);
}

void Descriptor::AddUpdateInfo(uint32_t set, uint32_t binding, std::vector<Sampler> samplers)
{
    std::vector<VkDescriptorImageInfo> imageInfos(samplers.size());
    for (int i = 0; i < samplers.size(); i++)
    {
        imageInfos[i] = VkDescriptorImageInfo{
            .sampler = samplers[i]._sampler,
        };
    }
    writesFieldsMap[{set, binding}].imageInfos = imageInfos;
    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = _descriptorSets[(int)set],
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = (uint32_t)samplers.size(),
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
        imageInfos[i] = VkDescriptorImageInfo{
            .sampler = samplers[i]._sampler,
            .imageView = views[i]._imageView,
            .imageLayout = views[i]._texture.imageLayout,
        };
    }
    writesFieldsMap[{set, binding}].imageInfos = imageInfos;
    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = _descriptorSets[(int)set],
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
    writesFieldsMap.clear();
}