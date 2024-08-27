//
// Created by piero on 4/08/2024.
//
// #include <iostream>
// #include <vector>
// #include <string>
// #include <optional>
// #include <algorithm>
// #include <volk.h>
// #include "Lettuce/Core/Device.hpp"
// #include "Lettuce/Core/Utils.hpp"
// #include "Lettuce/Core/CommandPool.hpp"
// #include "Lettuce/Core/CommandList.hpp"

// using namespace Lettuce::Core;

// void CommandPool::Build(Device &device, QueueType queueType, uint32_t count)
// {
//     _device = device;
//     _count = (std::max)((int)count, 1);

//     VkCommandPoolCreateInfo cmdPoolCI = {
//         .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
//         .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
//     };
//     if (queueType == QueueType::Graphics)
//     {
//         cmdPoolCI.queueFamilyIndex = _device._gpu.graphicsFamily.value();
//     }
//     checkResult(vkCreateCommandPool(_device._device, &cmdPoolCI, nullptr, &_commandPool), "CommandPool created successfully");
//     AllocateCommandBuffers();
// }

// std::vector<CommandList> CommandPool::GetCommandLists()
// {
//     std::vector<CommandList> cmds(_count);

//     for (int i = 0; i < _count; i++)
//     {
//         CommandList cmd;
//         cmd._commandBuffer = _commandBuffers[i];
//         cmd.Create(_device);
//         cmds[i] = cmd;
//     }

//     return cmds;
// }

// void CommandPool::AllocateCommandBuffers()
// {
//     VkCommandBufferAllocateInfo cmdBufferAI = {
//         .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
//         .commandPool = _commandPool,
//         .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
//         .commandBufferCount = _count,
//     };
//     _commandBuffers.resize(_count);
//     checkResult(vkAllocateCommandBuffers(_device._device, &cmdBufferAI, _commandBuffers.data()), "CommandBuffers allocated successfully");
// }

// void CommandPool::Reset()
// {
//     vkResetCommandPool(_device._device, _commandPool, VkCommandPoolResetFlagBits::VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
// }

// void CommandPool::Destroy()
// {
//     vkFreeCommandBuffers(_device._device, _commandPool, _count, _commandBuffers.data());
//     vkDestroyCommandPool(_device._device, _commandPool, nullptr);
// }