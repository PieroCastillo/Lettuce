//
// Created by piero on 29/12/2024.
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <vector>
#include "Lettuce/Core/IResource.hpp"
#include "Lettuce/Core/BufferResource.hpp"

using namespace Lettuce::Core;

void BufferResource::Create(const std::shared_ptr<Device> &device, uint32_t size, VkBufferUsageFlags usage)
{
    _device = device;

    VkBufferCreateInfo bufferCI = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    checkResult(vkCreateBuffer(device->_device, &bufferCI, nullptr, &_buffer));
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
std::shared_ptr<IResource> BufferResource::GetReference()
{
    return std::shared_ptr<IResource>(this);
}