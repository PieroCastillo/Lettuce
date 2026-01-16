// standard headers
#include <limits>
#include <memory>
#include <vector>
#include <algorithm>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Core/common.hpp"

using namespace Lettuce::Core;

/*
Indirect Set Layout:
|                  Allocation                  |
|                    Buffer                    |
| count | VkABCIndirectCommand | UserData |... |
*/

IndirectSet Device::CreateIndirectSet(const IndirectSetDesc& desc)
{
    auto dev = impl->m_device;
    auto gpu = impl->m_physicalDevice;
    VkDeviceMemory mem;
    VkBuffer buffer;

    uint32_t vkStructSize = 0;
    uint32_t userDataSize = desc.userDataSize;
    switch (desc.type)
    {
    case IndirectType::Draw:        vkStructSize = sizeof(VkDrawIndirectCommand); break;
    case IndirectType::DrawIndexed: vkStructSize = sizeof(VkDrawIndexedIndirectCommand); break;
    case IndirectType::DrawMesh:    vkStructSize = sizeof(VkDrawMeshTasksIndirectCommandEXT); break;
        // VkCmdDispatchIndirect doesn't support stride
    case IndirectType::Dispatch:    vkStructSize = sizeof(VkDispatchIndirectCommand); userDataSize = 0;  break;
    }

    // for CmdDraw/DrawIndexed/DrawMeshTasks/Dispatch Indirect, 4 is the alignment
    uint32_t stride = align_up(vkStructSize + userDataSize, 4);
    uint32_t bufferSize = sizeof(uint32_t) + (desc.maxCount * stride);

    // create buffer
    VkBufferCreateInfo bufferCI = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    handleResult(vkCreateBuffer(dev, &bufferCI, nullptr, &buffer));

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(dev, buffer, &memReqs);
    bufferSize = memReqs.size;

    // create memory allocation
    VkMemoryAllocateFlagsInfo memAllocFlags = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
        .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
        .deviceMask = 0,
    };
    VkMemoryAllocateInfo memAlloc = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = &memAllocFlags,
        .allocationSize = bufferSize,
        .memoryTypeIndex = findMemoryTypeIndex(dev, gpu, memReqs.memoryTypeBits, MemoryAccess::GPUOnly),
    };
    handleResult(vkAllocateMemory(dev, &memAlloc, nullptr, &mem));

    handleResult(vkBindBufferMemory(dev, buffer, mem, 0));

    // TODO: impl
    return impl->indirectSets.allocate({ mem, buffer, desc.type, stride });
}

void Device::Destroy(IndirectSet indirectSet)
{
    auto& info = impl->indirectSets.get(indirectSet);

    vkDestroyBuffer(impl->m_device, info.indirectSetBuffer, 0);
    vkFreeMemory(impl->m_device, info.indirectSetMemory, 0);
    impl->indirectSets.free(indirectSet);
}

MemoryView Device::GetIndirectSetView(IndirectSet indirectSet)
{
    auto& info = impl->indirectSets.get(indirectSet);
    // only size and gpuAddress are required, TODO: maybe impl a buffer?
    return { info.size, 0, info.gpuAddress, {0,0}, 0 };
}