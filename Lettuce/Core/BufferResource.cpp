//
// Created by piero on 29/12/2024.
//
#include "Lettuce/Core/Common.hpp"
#include <iostream>
#include <vector>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/IResource.hpp"
#include "Lettuce/Core/BufferResource.hpp"

using namespace Lettuce::Core;

BufferResource::BufferResource(const std::shared_ptr<Device> &device, std::vector<BufferBlock> bufferBlocks)
    : _device(device),
      _blocks(bufferBlocks)
{
    uint32_t size = 0;
    VkBufferUsageFlags usage = 0;
    for (const auto &block : bufferBlocks)
    {
        if (block.size == 0)
            continue;

        size += block.size;
        usage |= block.usage;
    }

    VkBufferCreateInfo bufferCI = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    checkResult(vkCreateBuffer(device->GetHandle(), &bufferCI, nullptr, GetHandlePtr()));
    _size = size;

    VkBufferDeviceAddressInfo addressInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = GetHandle(),
    };
    address = (uint64_t)vkGetBufferDeviceAddress(_device->GetHandle(), &addressInfo);
}

BufferResource::BufferResource(const std::shared_ptr<Device> &device, uint32_t size, VkBufferUsageFlags2 usage) : _device(device)
{
    _size = size;

    VkBufferCreateInfo bufferCI = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    if (device->GetEnabledRecommendedFeatures().maintenance5)
    {
        VkBufferUsageFlags2CreateInfo flags = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO,
            .usage = usage,
        };
        bufferCI.pNext = &flags;
    }
    else
    {
        bufferCI.usage = (VkBufferUsageFlags)usage;
    }

    checkResult(vkCreateBuffer(device->GetHandle(), &bufferCI, nullptr, GetHandlePtr()));
}

void BufferResource::Release()
{
    vkDestroyBuffer(_device->GetHandle(), GetHandle(), nullptr);
}

uint64_t BufferResource::GetAddress()
{
    return address;
}

ResourceType BufferResource::GetResourceType()
{
    return ResourceType::Buffer;
}

ResourceLinearity BufferResource::GetResourceLinearity()
{
    return ResourceLinearity::Linear;
}

VkMemoryRequirements BufferResource::GetResourceMemoryRequirements()
{
    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(_device->GetHandle(), GetHandle(), &memReqs);
    return memReqs;
}