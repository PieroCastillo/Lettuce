//
// Created by piero on 4/08/2024.
//
module;
#include <iostream>
#include <vector>
#include <string>
#include <optional>
#define VOLK_IMPLEMENTATION
#include "volk.h"

export module Lettuce:CommandPool;

import :Device;
import :Utils;
import :CommandList;

export namespace Lettuce::Core
{
    class CommandPool
    {
    public:
        Device _device;
        VkCommandPool _commandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> _commandBuffers;
        uint32_t _count;

        void Build(Device &device, QueueType queueType = QueueType::Graphics, uint32_t count = 1)
        {
            _device = device;
            _count = std::max(count, 1U);

            VkCommandPoolCreateInfo cmdPoolCI = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            };
            if (queueType == QueueType::Graphics)
            {
                cmdPoolCI.queueFamilyIndex = _device._gpu.graphicsFamily.value();
            }
            checkResult(vkCreateCommandPool(_device._device, &cmdPoolCI, nullptr, &_commandPool), "CommandPool created successfully");
            AllocateCommandBuffers();
        }

        std::vector<CommandList> GetCommandLists()
        {
            std::vector<CommandList> cmds(_count);

            for (int i = 0; i < _count; i++)
            {
                CommandList cmd;
                cmd._commandBuffer = _commandBuffers[i];
                cmd.Create(_device);
                cmds[i] = cmd;
            }

            return cmds;
        }

        void AllocateCommandBuffers()
        {
            VkCommandBufferAllocateInfo cmdBufferAI = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = _commandPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = _count,
            };
            _commandBuffers.resize(_count);
            checkResult(vkAllocateCommandBuffers(_device._device, &cmdBufferAI, _commandBuffers.data()), "CommandBuffers allocated successfully");
        }

        void Reset()
        {
            vkResetCommandPool(_device._device, _commandPool, VkCommandPoolResetFlagBits::VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
        }

        void Destroy()
        {
            vkFreeCommandBuffers(_device._device, _commandPool, _count, _commandBuffers.data());
            vkDestroyCommandPool(_device._device, _commandPool, nullptr);
        }
    };
}