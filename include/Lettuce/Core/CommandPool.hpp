//
// Created by piero on 4/08/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <volk.h>
#include "Device.hpp"
#include "Utils.hpp"
#include "CommandList.hpp"

namespace Lettuce::Core
{
    class CommandPool
    {
    public:
        Device _device;
        VkCommandPool _commandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> _commandBuffers;
        uint32_t _count;

        void Build(Device &device, QueueType queueType = QueueType::Graphics, uint32_t count = 1);

        std::vector<CommandList> GetCommandLists();

        void AllocateCommandBuffers();

        void Reset();

        void Destroy();
    };
}