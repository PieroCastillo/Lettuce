//
// Created by piero on 22/12/2024.
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include <numeric>
#include <limits>
#include <bitset>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/ResourcePool.hpp"
#include "Lettuce/Core/IResource.hpp"
#include "Lettuce/Core/BufferResource.hpp"
#include "Lettuce/Core/ImageResource.hpp"
#include "Lettuce/Core/Utils.hpp"

using namespace Lettuce::Core;

void ResourcePool::Map(uint32_t offset, uint32_t size)
{
    vkMapMemory(_device->_device, _memory, offset, size, 0, &temp);
}
void ResourcePool::SetData(void *data, uint32_t offset, uint32_t size)
{
    memcpy(temp, data, size);
}
void ResourcePool::UnMap()
{
    vkUnmapMemory(_device->_device, _memory);
}

/*
to measure the required size of a ResourcePool / VkDeviceMemory,
we use the next expression:
total_size = size[0] + sumatory from i = 1 to n-1 of:
    M ( lcm(granularity, alignment[i-1]) , size[i-1])
where:
n : # resources
M(x,y) := lower multiple of x major than y
the code is:

if resource count = 1
    ...
else
    if
        ...
    else
        for resources[1] to resources[n-1]
            measuredSize += M(lcm(granularity, resource[i].alignment), size[i-1])

        measuredSize += resources[n-1].size
*/
void ResourcePool::Bind(const std::shared_ptr<Device> &device, VkMemoryPropertyFlags requiredFlags, uint32_t requiredMemoryTypeBits)
{
    _device = device;
    uint64_t measuredSize = 0;
    uint32_t memoryTypeIndex;
    std::vector<VkBindBufferMemoryInfo> bufferBindInfos;
    std::vector<VkBindImageMemoryInfo> imageBindInfos;
    // measure

    // fill with memory requirements of resources
    // we do this to avoid get memory requirements 2 times of the same resource
    std::vector<VkMemoryRequirements> memoryReqs;
    memoryReqs.reserve(resourcePtrs.size());
    std::vector<uint32_t> offsets;
    offsets.reserve(resourcePtrs.size());

    for (int i = 0; i < resourcePtrs.size(); i++)
    {
        memoryReqs.push_back(resourcePtrs[i]->GetResourceMemoryRequirements());
    }
    offsets.push_back(0);
    if (resourcePtrs.size() > 1)
    {
        measuredSize = memoryReqs[0].size;
        resourcePtrs[0]->offset = 0;
    }
    else
    {
        uint32_t granularity = 1;
        uint32_t mod = 0;
        ResourceLinearity lastType = resourcePtrs[0]->GetResourceLinearity();

        // here we do the "real" measure
        for (int i = 1; i < resourcePtrs.size(); i++)
        {
            if (lastType != resourcePtrs[i]->GetResourceLinearity())
                granularity = device->_gpu.bufferImageGranularity;
            else
                granularity = 1;

            mod = std::lcm(granularity, memoryReqs[i].alignment);
            resourcePtrs[i]->offset = measuredSize + 1; // offset is size+1
            /*
                |=====| |=============| |==========|
                0     n n+1           s s+1        ....
                offset  offset          offset
            */

            measuredSize += (mod + memoryReqs[i - 1].size - 1) & ~(mod - 1);
            offsets.push_back(measuredSize);
        }
        measuredSize += memoryReqs[resourcePtrs.size() - 1].size;
    }

    // find memory type
    // here we are going to execute an "and" operator for every memory type bits in memory reqs
    // if result == 0, throws error
    // else, choose the selected memmory type, with preference of
    // memory types with device_local_bit flag
    uint32_t memTypeBits = std::numeric_limits<uint32_t>::max();

    for (int i = 0; i < resourcePtrs.size(); i++)
    {
        memTypeBits = memTypeBits & memoryReqs[i].memoryTypeBits & requiredMemoryTypeBits;
    }

    if (memTypeBits == 0)
    {
        throw std::runtime_error("Memory Bind Error: Resources should have at least 1 memory type bit at common.");
    }

    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(device->_pdevice, &memoryProperties);

    std::vector<int> suitableMemoryTypeIndices;
    suitableMemoryTypeIndices.reserve(memoryProperties.memoryTypeCount); // max count of memory types

    for (int i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((memTypeBits >> i) & 1) // check if the i-th bit of memTypeBits is true
        {
            suitableMemoryTypeIndices.push_back(i);
        }
    }

    // select memory type with the required memory flags
    bool exists = false;
    for (int i = 0; i < suitableMemoryTypeIndices.size(); i++)
    {
        if (memoryProperties.memoryTypes[suitableMemoryTypeIndices[i]].propertyFlags & requiredFlags)
        {
            memoryTypeIndex = i;
            exists = true;
        }
    }

    if (!exists)
        throw std::runtime_error("Memory Bind Error: There's no memory type with the required flags.");

    // create allocation
    VkMemoryAllocateInfo memoryAI = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = measuredSize,
        .memoryTypeIndex = memoryTypeIndex,
    };

    checkResult(vkAllocateMemory(device->_device, &memoryAI, nullptr, &_memory));

    // create bind infos
    int j = 0;
    bool start = true;
    for (const auto &resourcePtr : resourcePtrs)
    {
        switch (resourcePtr->GetResourceType())
        {
        case ResourceType::Buffer:
        {
            VkBindBufferMemoryInfo bindBufferI = {
                .sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO,
                .buffer = std::dynamic_pointer_cast<BufferResource>(resourcePtr)->_buffer,
                .memory = _memory,
                .memoryOffset = offsets[j],
            };
            bufferBindInfos.push_back(bindBufferI);
            break;
        }
        case ResourceType::Image:
        {
            VkBindImageMemoryInfo bindImageI = {
                .sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO,
                .image = std::dynamic_pointer_cast<ImageResource>(resourcePtr)->_image,
                .memory = _memory,
                .memoryOffset = offsets[j],
            };
            imageBindInfos.push_back(bindImageI);
            break;
        }
        default:
            break;
        }
        start = false;
        j++;
    }

    // bind resources
    if (bufferBindInfos.size() > 0)
    {
        checkResult(vkBindBufferMemory2(device->_device, (uint32_t)bufferBindInfos.size(), bufferBindInfos.data()));
    }
    if (imageBindInfos.size() > 0)
    {
        checkResult(vkBindImageMemory2(device->_device, (uint32_t)imageBindInfos.size(), imageBindInfos.data()));
    }
}

void ResourcePool::AddResource(const std::shared_ptr<IResource> &resourcePtr)
{
    resourcePtrs.push_back(resourcePtr);
}

void ResourcePool::Release()
{
    vkFreeMemory(_device->_device, _memory, nullptr);
}