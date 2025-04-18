//
// Created by piero on 29/12/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "Device.hpp"
#include "IResource.hpp"
#include "IReleasable.hpp"
#include "IManageHandle.hpp"

namespace Lettuce::Core
{
    struct BufferBlock
    {
        uint32_t offset;
        uint32_t size;
        VkBufferUsageFlagBits usage;
    };

    class BufferResource : public IResource, public IReleasable, public IManageHandle<VkBuffer>
    {
    private:
        uint64_t address = 0;
    public:
        uint32_t _size;
        std::shared_ptr<Device> _device;
        std::vector<BufferBlock> _blocks;
        BufferResource(const std::shared_ptr<Device> &device, std::vector<BufferBlock> bufferBlocks);
        BufferResource(const std::shared_ptr<Device> &device, uint32_t size, VkBufferUsageFlags2 usage);
        void Release();
        uint64_t GetAddress();
        ResourceType GetResourceType();
        ResourceLinearity GetResourceLinearity();
        VkMemoryRequirements GetResourceMemoryRequirements();
    };
}