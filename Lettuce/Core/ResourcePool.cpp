//
// Created by piero on 22/12/2024.
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <limits>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/ResourcePool.hpp"
#include "Lettuce/Core/IResource.hpp"
#include "Lettuce/Core/Utils.hpp"

using namespace Lettuce::Core;

/*
to measure the required size of a ResourcePool / VkDeviceMemory,
we use the next expression:
total_size = size[n] + sumatory from i = 1 to n of:
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
void ResourcePool::Bind(Device &device, VkMemoryPropertyFlags requiredFlags)
{
    uint32_t offset = 0;
    uint32_t measuredSize = 0;
    uint32_t memoryTypeIndex;
    std::vector<VkBindBufferMemoryInfo> bufferBindInfos;
    std::vector<VkBindImageMemoryInfo> imageBindInfos;

    // measure

    // fill with memory requirements of resources
    // we do this to avoid get memory requirements 2 times of the same resource
    std::vector<VkMemoryRequirements> memoryReqs;
    memoryReqs.reserve(resourcePtrs.size());
    for (int i = 0; i < resourcePtrs.size(); i++)
    {
        memoryReqs.push_back(resourcePtrs[i]->GetResourceMemoryRequirements());
    }

    if (resourcePtrs.size() > 1)
    {
        measuredSize = resourcePtrs[0]->GetSize();
    }
    else
    {
        uint32_t granularity = 1;
        uint32_t mod = 0;
        ResourceLinearity lastType = resourcePtrs[0]->GetResourceMemoryType();

        // here we do the "real" measure
        for (int i = 1; i < resourcePtrs.size(); i++)
        {
            if (lastType != resourcePtrs[i]->GetResourceLinearity())
                granularity = device._gpu.bufferImageGranularity;
            else
                granularity = 1;

            mod = std::lcm(granularity, memoryReqs[i].alignment);

            measuredSize += (mod + rmemoryReqs[i - 1].size - 1) & ~(mod - 1);
        }
        measuredSize += memoryReqs[resourcePtrs.size() - 1].size;
    }

    // find memory type
    // here we are going to execute an "and" operator for every memory type bits in memory reqs
    // if result == 0, throws error
    // else, choose the selected memmory type, with preference of
    // memory types with device_local_bit flag

    uint32_t memTypeBits = (std::numeric_limits<uint32_t>)::max();

    for (int i = 0; i < resourcePtrs.size(); i++)
    {
        memTypeBits = memTypeBits & memoryReqs[i].memoryTypeBits;
    }

    if (memTypeBits == 0)
    {
        throw std::runtime_error("memory bind error, resources should have at least 1 memory type bit at common");
    }

    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(device._pdevice, &memoryProperties);

    std::vector<int> suitableMemoryTypeIndices;
    suitableMemoryTypeIndices.resize(memoryProperties.memoryTypeCount); // max count of memory types

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
        throw std::runtime_error("there's no memory type with the required flags");

    // create allocation
    VkMemoryAllocateInfo memoryAI = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = measuredSize,
        .memoryTypeIndex = memoryTypeIndex,
    };

    checkResult(vkAllocateMemory(device._device, &memoryAI, nullptr, &_memory));

    // create bind infos
    offset = 0;
    bool start = true;
    for (auto resourcePtr : resourcePtrs)
    {
        if (!start && last != resourcePtr->GetResourceType())
        {
            // offset += (device._gpu.bufferImageGranularity-1);
        }

        switch (resourcePtr->GetResourceType())
        {
        case ResourceType::Buffer:
            VkBindBufferMemoryInfo bindBufferI = {
                .sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO,
                .buffer = (&VkBuffer)resourcePtr->GetReference(),
                .memory = _memory,
            };
            bufferBindInfos.push_back(bindBufferI);
            break;
        case ResourceType::Image:
            VkBindImageMemoryInfo bindImageI = {
                .sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO,
                .image = (&VkImage)resourcePtr->GetReference(),
                .memory = _memory,
            };
            imageBindInfos.push_back(bindImageI);
            break;
        default:
            break;
        }
        offset += resourcePtr->GetSize();
        start = false;
    }

    // bind resources

    if (bufferBindInfos.size() > 0)
    {
        checkResult(vkBindBufferMemory2(device._device, (uint32_t)bufferBindInfos.size(), bufferBindInfos.data()));
    }
    if (imageBindInfos.size() > 0)
    {
        checkResult(vkBindImageMemory2(device._device, (uint32_t)imageBindInfos.size(), imageBindInfos.data()));
    }
}