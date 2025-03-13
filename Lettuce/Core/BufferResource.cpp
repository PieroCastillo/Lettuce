//
// Created by piero on 29/12/2024.
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <vector>
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
    checkResult(vkCreateBuffer(device->_device, &bufferCI, nullptr, &_buffer));
    _size = size;
}

BufferResource::BufferResource(const std::shared_ptr<Device> &device, uint32_t size, VkBufferUsageFlags usage) : _device(device)
{
    _size = size;
    VkBufferCreateInfo bufferCI = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    
    checkResult(vkCreateBuffer(device->_device, &bufferCI, nullptr, &_buffer));
}

void BufferResource::Release()
{
    vkDestroyBuffer(_device->_device, _buffer, nullptr);
}

uint64_t BufferResource::GetAddress()
{
    VkBufferDeviceAddressInfo addressInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = _buffer,
    };
    return (uint64_t)vkGetBufferDeviceAddress(_device->_device, &addressInfo);
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
    vkGetBufferMemoryRequirements(_device->_device, _buffer, &memReqs);
    return memReqs;
}