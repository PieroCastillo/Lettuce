//
// Created by piero on 29/11/2024.
//
#pragma once
#include "common.hpp"
#include <vector>
#include <tuple>

namespace Lettuce::Core
{
    typedef std::vector<std::tuple<int,uint32_t, VkBufferUsageFlags>> VirtualBuffers;
    /// @brief Represent a segment of memory
    class MemoryBlock
    {
        private:
        VirtualBuffers buffers;
        public:
        Device _device;
        VkDeviceMemory _allocation;
        VkBuffer _buffer;

        void AddVirtualBuffer(int index, uint32_t size, VkBufferUsageFlags usage);

        void Build(Device& device);
        void Destroy();
    };
}