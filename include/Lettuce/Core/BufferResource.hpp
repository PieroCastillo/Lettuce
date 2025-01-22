//
// Created by piero on 29/12/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "IResource.hpp"
#include "Device.hpp"

namespace Lettuce::Core
{
    struct BufferBlock
    {
        uint32_t offset;
        uint32_t size;
    };

    class BufferResource : public IResource
    {
    public:
        VkBuffer _buffer;
        std::shared_ptr<Device> _device;
        std::vector<BufferBlock> _blocks;
        void Create(const std::shared_ptr<Device>& device, std::vector<BufferBlock> bufferBlocks, VkBufferUsageFlags usage);
        void Create(const std::shared_ptr<Device>& device, uint32_t size, VkBufferUsageFlags usage);
        ResourceType GetResourceType();
        ResourceLinearity GetResourceLinearity();
        VkMemoryRequirements GetResourceMemoryRequirements();
        std::shared_ptr<IResource> GetReference();
    };
}