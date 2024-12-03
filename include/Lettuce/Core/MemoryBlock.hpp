//
// Created by piero on 29/11/2024.
//
#pragma once
#include "common.hpp"
#include <vector>
#include <tuple>
#include "Device.hpp"

namespace Lettuce::Core
{
    struct SubblockInfo{
        uint32_t size;
        uint32_t offset;
    };

    typedef std::vector<std::tuple<uint32_t, VkBufferUsageFlags>> VirtualBuffers;
    /// @brief Represent a contigous block of memory
    class MemoryBlock
    {
        private:
        VirtualBuffers buffers;
        VkBufferUsageFlags usage;
        public:
        Device _device;
        VmaAllocation _allocation;
        VkBuffer _buffer;

        void Reserve(int size);
        void AddSubblock(int index, uint32_t size, VkBufferUsageFlags usage);
        SubblockInfo GetSubblockInfo(int index);

        void Build(Device& device, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocation);
        void Destroy();

        static void TransferSubblock(MemoryBlock &blockSrc, MemoryBlock &blockDst);
    };
}