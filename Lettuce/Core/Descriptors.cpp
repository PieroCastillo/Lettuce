//
// Created by piero on 26/06/2024
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/Sampler.hpp"
#include "Lettuce/Core/Descriptors.hpp"
#include "Lettuce/Core/BufferResource.hpp"
#include "Lettuce/Core/ImageViewResource.hpp"

using namespace Lettuce::Core;

void Descriptors::AddBinding(uint32_t set,
                             uint32_t binding,
                             DescriptorType type,
                             PipelineStage stage,
                             uint32_t descriptorCount)
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

void Descriptors::Assemble(uint32_t maxSets)
{
    // create descriptor set layouts
    int index = 0;
    _layouts.resize(bindingsMap.size());
    for (auto &[_, bindings] : bindingsMap)
    {
        // for each binding increments +1 per type
        int j = 0;
        for (auto binding : bindings)
        {
            typesMap[binding.descriptorType] += binding.descriptorCount;
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
        checkResult(vkCreateDescriptorSetLayout(_device->_device, &layoutCI, nullptr, &layout));
        _layouts[index] = layout;
        index++;
    }

    // creates the sizes required for the Descriptors Pool
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
    checkResult(vkCreateDescriptorPool(_device->_device, &poolCI, nullptr, &_pool));

    // creates descriptor set
    _descriptorSets.resize(_layouts.size(), VK_NULL_HANDLE);
    VkDescriptorSetAllocateInfo descriptorSetAI = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = _pool,
        .descriptorSetCount = (uint32_t)_descriptorSets.size(),
        .pSetLayouts = _layouts.data(),
    };
    checkResult(vkAllocateDescriptorSets(_device->_device, &descriptorSetAI, _descriptorSets.data()));
}

Descriptors::Descriptors(const std::shared_ptr<Device> &device) : _device(device)
{
}

void Descriptors::Release()
{
    checkResult(vkResetDescriptorPool(_device->_device, _pool, 0));
    vkDestroyDescriptorPool(_device->_device, _pool, nullptr);
    for (auto layout : _layouts)
    {
        vkDestroyDescriptorSetLayout(_device->_device, layout, nullptr);
    }
    bindingsMap.clear();
    _layouts.clear();
    typesMap.clear();
    sizes.clear();
}

void Descriptors::AddUpdateInfo(uint32_t set, uint32_t binding, const std::vector<std::pair<uint32_t, std::shared_ptr<BufferResource>>> &sizeBuffersPairs)
{
    std::vector<VkDescriptorBufferInfo> bufferInfos(sizeBuffersPairs.size());

    int index = 0;
    for (auto &[size, buffer] : sizeBuffersPairs)
    {
        VkDescriptorBufferInfo bufferI = {
            .buffer = buffer->_buffer,
            .offset = 0,
            .range = size,
        };
        bufferInfos[index] = bufferI;
        index++;
    }
    // this is needed for disabling the auto-deleting of the bufferInfos vector
    writesFieldsMap[{set, binding}].bufferInfos = bufferInfos;

    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = _descriptorSets[(int)set],
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = (uint32_t)sizeBuffersPairs.size(),
        .descriptorType = bindingsTypes[{set, binding}],
        .pBufferInfo = writesFieldsMap[{set, binding}].bufferInfos.data(),
    };
    writes.push_back(write);
}

void Descriptors::AddUpdateInfo(uint32_t set, uint32_t binding, const std::vector<std::shared_ptr<Sampler>> &samplers)
{
    std::vector<VkDescriptorImageInfo> imageInfos(samplers.size());
    for (int i = 0; i < samplers.size(); i++)
    {
        imageInfos[i] = VkDescriptorImageInfo{
            .sampler = samplers[i]->_sampler,
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
        .pImageInfo = writesFieldsMap[{set, binding}].imageInfos.data(),
    };
    writes.push_back(write);
}

void Descriptors::AddUpdateInfo(uint32_t set, uint32_t binding, const std::vector<std::pair<std::shared_ptr<Sampler>, std::shared_ptr<ImageViewResource>>> &samplerViewsPairs)
{
    std::vector<VkDescriptorImageInfo> imageInfos(samplerViewsPairs.size());

    int i = 0;
    for (auto &[sampler, view] : samplerViewsPairs)
    {
        VkDescriptorImageInfo imageInfo = {
            .sampler = sampler->_sampler,
            .imageView = view->_imageView,
            .imageLayout = view->_image->_layout,
        };
        imageInfos[i] = imageInfo;
        i++;
    }
    writesFieldsMap[{set, binding}].imageInfos = imageInfos;
    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = _descriptorSets[(int)set],
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = (uint32_t)samplerViewsPairs.size(),
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = writesFieldsMap[{set, binding}].imageInfos.data(),
    };
    writes.push_back(write);
}

void Descriptors::Update()
{
    vkUpdateDescriptorSets(_device->_device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
    writes.clear();
    writesFieldsMap.clear();
}