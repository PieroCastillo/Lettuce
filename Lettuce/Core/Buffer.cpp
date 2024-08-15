//
// Created by piero on 26/06/2024
//
#include <iostream>
#include <vector>
#define VOLK_IMPLEMENTATION
#include <volk.h>
#include <otherUtils.h>
#include <vma/vk_mem_alloc.h>

void Lettuce::Core::Buffer::Create(Device &device, uint32_t size, BufferUsage usage,
                                   VmaAllocationCreateFlags allocationFlags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                                   VmaMemoryUsage memoryUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO)
{
    _device = device;
    _size = size;
    _usage = usage;

    if (usage == BufferUsage::TransferSrc)
    {
        VkCommandPoolCreateInfo poolCI = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = _device._gpu.graphicsFamily.value(),
        };
        checkResult(vkCreateCommandPool(_device._device, &poolCI, nullptr, &_pool), "CommandPool of Buffer created sucessfully");
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

    checkResult(vmaCreateBuffer(_device.allocator, &bufferCI, &allocationCI, &_buffer, &_allocation, nullptr), "buffer created sucessfully");
}

/// @brief Maps data from host memory to buffer
/// @param src pinter to data to be mapped
void Lettuce::Core::Buffer::SetData(void *src)
{
    void *data;
    checkResult(vmaMapMemory(_device.allocator, _allocation, &data), "mapped sucessfully");
    memcpy(data, src, _size);
    vmaUnmapMemory(_device.allocator, _allocation);
    checkResult(vmaFlushAllocation(_device.allocator, _allocation, 0, _size), "flushed");
}

/// @brief Copy data from host visible Buffer to device prefered Buffer,
/// the usage of this function is for staging buffers creation only, another
/// usage may have unexpected behaviors.
/// @param buffer the dst buffer
void Lettuce::Core::Buffer::CopyTo(Buffer buffer)
{
    VkCommandBufferAllocateInfo commandBufferAI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = _pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(_device._device, &commandBufferAI, &cmd);

    VkCommandBufferBeginInfo cmdBeginI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(cmd, &cmdBeginI);

    VkBufferCopy bufferC = {
        .size = _size,
    };
    vkCmdCopyBuffer(cmd, _buffer, buffer._buffer, 1, &bufferC);

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitI = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };
    vkQueueSubmit(_device._graphicsQueue, 1, &submitI, nullptr);

    vkQueueWaitIdle(_device._graphicsQueue);

    vkFreeCommandBuffers(_device._device, _pool, 1, &cmd);
}

template <typename T>
static Lettuce::Core::Buffer Lettuce::Core::Buffer::CreateVertexBuffer(Device &device, std::vector<T> vertices)
{
    return CreateBufferWithStaging(device, BufferUsage::VertexBuffer, sizeof(vertices[0]) * vertices.size(), vertices.data());
}

template <typename T, typename = std::enable_if_t<
                          std::is_integral<T>::value && std::is_unsigned<T>::value>>
static Lettuce::Core::Buffer Lettuce::Core::Buffer::CreateIndexBuffer(Device &device, std::vector<T> indices)
{
    return CreateBufferWithStaging(device, BufferUsage::IndexBuffer, sizeof(indices[0]) * indices.size(), indices.data());
}

static Lettuce::Core::Buffer Lettuce::Core::Buffer::CreateBufferWithStaging(Device &device, BufferUsage bufferDstUsage, uint32_t size, void *data)
{
    Buffer stagingBuffer;
    Buffer buffer;
    stagingBuffer.Create(device, size, BufferUsage::TransferSrc,
                         VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
    stagingBuffer.SetData(data);

    auto usage = BufferUsage::TransferDst;
    usage |= bufferDstUsage;
    buffer.Create(device, size, usage, VMA_ALLOCATION_CREATE_MAPPED_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    stagingBuffer.CopyTo(buffer);
    stagingBuffer.Destroy();
    return buffer;
}

template <typename T>
static Lettuce::Core::Buffer Lettuce::Core::Buffer::CreateUniformBuffer(Device &device, T **data)
{
    Buffer buffer;
    buffer.Create(device, sizeof(T), BufferUsage::UniformBuffer, VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
    return buffer;
}

void Lettuce::Core::Buffer::Destroy()
{
    vmaDestroyBuffer(_device.allocator, _buffer, _allocation);

    if (_usage == BufferUsage::TransferSrc)
    {
        vkDestroyCommandPool(_device._device, _pool, nullptr);
    }
}
