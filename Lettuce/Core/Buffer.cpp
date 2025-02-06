//
// Created by piero on 26/06/2024
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <vector>
#include "Lettuce/Core/Buffer.hpp"
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Utils.hpp"

using namespace Lettuce::Core;

Buffer::Buffer(const std::shared_ptr<Device> &device, uint32_t size, BufferUsage usage,
       VmaAllocationCreateFlags allocationFlags,
       VmaMemoryUsage memoryUsage) : _device(device),
                                     _size(size),
                                     _usage(usage)
{
    if (usage == BufferUsage::TransferSrc)
    {
        VkCommandPoolCreateInfo poolCI = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = _device->_gpu.graphicsFamily.value(),
        };
        checkResult(vkCreateCommandPool(_device->_device, &poolCI, nullptr, &_pool));
    }

    VkBufferCreateInfo bufferCI = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = (VkBufferUsageFlags)usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VmaAllocationCreateInfo allocationCI = {
        .flags = allocationFlags,
        .usage = memoryUsage,
    };

    checkResult(vmaCreateBuffer(_device->allocator, &bufferCI, &allocationCI, &_buffer, &_allocation, nullptr));
}

void Buffer::Map()
{
    checkResult(vmaMapMemory(_device->allocator, _allocation, &data));
}

void Buffer::SetData(void *src)
{
    memcpy(data, src, _size);
}

void Buffer::Unmap()
{
    vmaUnmapMemory(_device->allocator, _allocation);
    checkResult(vmaFlushAllocation(_device->allocator, _allocation, 0, _size));
}

void Buffer::CopyTo(const std::shared_ptr<Buffer>& buffer)
{
    VkCommandBufferAllocateInfo commandBufferAI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = _pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(_device->_device, &commandBufferAI, &cmd);

    VkCommandBufferBeginInfo cmdBeginI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(cmd, &cmdBeginI);

    VkBufferCopy bufferC = {
        .size = _size,
    };
    vkCmdCopyBuffer(cmd, _buffer, buffer->_buffer, 1, &bufferC);

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitI = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };
    vkQueueSubmit(_device->_graphicsQueues[0], 1, &submitI, nullptr);

    vkQueueWaitIdle(_device->_graphicsQueues[0]);

    vkFreeCommandBuffers(_device->_device, _pool, 1, &cmd);
}

std::shared_ptr<Buffer> Buffer::CreateBufferWithStaging(const std::shared_ptr<Device> &device, BufferUsage bufferDstUsage, uint32_t size, void *data)
{
    auto stagingBuffer = std::make_shared<Buffer>(device, size, BufferUsage::TransferSrc,
                                                  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
    stagingBuffer->Map();
    stagingBuffer->SetData(data);
    stagingBuffer->Unmap();

    auto usage = BufferUsage::TransferDst;
    usage |= bufferDstUsage;
    auto buffer = std::make_shared<Buffer>(device, size, usage, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    stagingBuffer->CopyTo(buffer);
    stagingBuffer.reset();
    return buffer;
}

Buffer::~Buffer()
{
    vmaDestroyBuffer(_device->allocator, _buffer, _allocation);

    if (_usage == BufferUsage::TransferSrc)
    {
        vkDestroyCommandPool(_device->_device, _pool, nullptr);
    }
}
