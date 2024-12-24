//
// Created by piero on 22/12/2024.
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <vector>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/ResourcePool.hpp"
#include "Lettuce/Core/IResource.hpp"
#include "Lettuce/Core/Utils.hpp"

using namespace Lettuce::Core;

void ResourcePool::Bind(Device &device)
{
    uint32_t offset = 0;
    uint32_t measuredSize = 0;
    uint32_t memoryTypeIndex;
    ResourceType last;
    std::vector<VkBindBufferMemoryInfo> bufferBindInfos;
    std::vector<VkBindImageMemoryInfo> imageBindInfos;

    // measure

    for (auto resourcePtr : resourcePtrs)
    {
        measuredSize += resourcePtr->GetSize();
        switch (resourcePtr->GetResourceType())
        {
        case ResourceType::Buffer:

            break;
        case ResourceType::Texture:

            break;
        default:
            break;
        }
    }

    // find memory type, analize aligment

    VkPhysicalDeviceMemoryProperties2 props;

    // vkGetPhysicalDeviceMemoryProperties2();

    // create allocation

    VkMemoryAllocateInfo memoryAI = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = measuredSize,
        .memoryTypeIndex = memoryTypeIndex,
    }

    checkResult(vkAllocateMemory(device._device, &memoryAI, nullptr, &_memory));

    // create bind infos
    offset = 0;
    bool start = true;
    for (auto resourcePtr : resourcePtrs)
    {
        if(!start && last != resourcePtr->GetResourceType())
        {
            //offset += (device._gpu.bufferImageGranularity-1);
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
        case ResourceType::Texture:
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