//
// Created by piero on 22/12/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include <limits>
#include "IResource.hpp"
#include "Device.hpp"
#include "IReleasable.hpp"

namespace Lettuce::Core
{
    class ResourcePool : public IReleasable
    {
    private:
        uint32_t poolSize;

    public:
        void *temp;
        std::shared_ptr<Device> _device;
        std::vector<std::shared_ptr<IResource>> resourcePtrs;
        VkDeviceMemory _memory;
        ResourcePool() {}
        void Release();
        void Map(uint32_t offset, uint32_t size);
        void SetData(void *data, uint32_t offset, uint32_t size);
        void UnMap();
        uint32_t GetSize() { return poolSize; }
        void AddResource(const std::shared_ptr<IResource> &resourcePtr);
        void Bind(const std::shared_ptr<Device> &device, VkMemoryPropertyFlags requiredFlags, uint32_t requiredMemoryTypeBits = (std::numeric_limits<uint32_t>::max)());
    };
}