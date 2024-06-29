//
// Created by piero on 26/06/2024
//
module;
#include <iostream>
#include <vector>
#define VOLK_IMPLEMENTATION
#include <volk.h>

export module Lettuce:Buffer;

import :Device;
import :Utils;

export namespace Lettuce::Core
{
    enum class MemoryProperty
    {
        DeviceLocal = 0x00000001,     // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        HostVisible = 0x00000002,     // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        HostCoherent = 0x00000004,    // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        HostCached = 0x00000008,      // VK_MEMORY_PROPERTY_HOST_CACHED_BIT
        LazilyAllocated = 0x00000010, // VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT
        Protected = 0x00000020,       // VK_MEMORY_PROPERTY_PROTECTED_BIT
    };

    enum class BufferUsage
    {
        TransferSrc = 0x00000001,
        TransferDst = 0x00000002,
        UniformTexelBuffer = 0x00000004,
        StorageTexelBuffer = 0x00000008,
        UniformBuffer = 0x00000010,
        StorageBuffer = 0x00000020,
        IndexBuffer = 0x00000040,
        VertexBuffer = 0x00000080,
        IndirectBuffer = 0x00000100,
        ShaderDeviceAdress = 0x00020000,
        VideoDecodeSrc = 0x00002000,
        VideoDecodeDst = 0x00004000,
        TransformFeedbackBuffer = 0x00000800,
        TransformFeedbackCounterBuffer = 0x00001000,
        ConditionalRendering = 0x00000200,
        AccelerationStructureBuildInputReadOnly = 0x00080000,
        AccelerationStructureStorage = 0x00100000,
        ShaderBindingTable = 0x00000400,
        VideoEncodeDst = 0x00008000,
        VideoEncodeSrc = 0x00010000,
        SamplerDescriptorBuffer = 0x00200000,
        ResourceDescriptorBuffer = 0x00400000,
        PushDescriptorsDescriptorBuffer = 0x04000000,
        MicromapBuildInputReadOnly = 0x00800000,
        MicromapStorage = 0x01000000,
    };

    class Buffer
    {
        Device _device;
        VkBuffer _buffer;
        VkDeviceMemory _memory;
        VkCommandPool _pool;
        uint32_t _size;

        void Create(Device &device, uint32_t size, BufferUsage usage, MemoryProperty properties)
        {
            _device = device;
            _size = size;
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

            checkResult(vkCreateBuffer(_device._device, &bufferCI, nullptr, &_buffer), "buffer created sucessfully");

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(_device._device, _buffer, &memRequirements);

            VkMemoryAllocateInfo memoryAI = {
                .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .allocationSize = memRequirements.size,
                .memoryTypeIndex = _device._gpu.FindMemoryType(memRequirements.memoryTypeBits, (VkMemoryPropertyFlags)properties),
            };

            checkResult(vkAllocateMemory(_device._device, &memoryAI, nullptr, &_memory), "memory allocated sucessfully");

            vkBindBufferMemory(_device._device, _buffer, _memory, 0);
        }

        void SetData(void *src)
        {
            void *data;
            vkMapMemory(_device._device, _memory, 0, _size, 0, &data);
            memcpy(data, src, _size);
            vkUnmapMemory(_device._device, _memory);
        }

        void CopyTo(Buffer buffer)
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
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
            vkBeginCommandBuffer(cmd, &cmdBeginI);

            VkBufferCopy bufferC = {
                .size = _size,
            };
            vkCmdCopyBuffer(cmd, _buffer, buffer._buffer, 1, &bufferC);

            vkEndCommandBuffer(cmd);

            VkSubmitInfo submitI = {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .commandBufferCount = 1,
                .pCommandBuffers = &cmd};
            vkQueueSubmit(_device._graphicsQueue, 1, &submitI, nullptr);
            vkQueueWaitIdle(_device._graphicsQueue);

            vkFreeCommandBuffers(_device._device, _pool, 1, &cmd);
        }

        void Destroy()
        {
            vkDestroyBuffer(_device._device, _buffer, nullptr);
            vkFreeMemory(_device._device, _memory, nullptr);
        }
    };
}