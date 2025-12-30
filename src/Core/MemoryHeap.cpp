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

MemoryHeap Device::CreateMemoryHeap(const MemoryHeapDesc& desc)
{
    auto device = impl->m_device;
    auto access = desc.cpuVisible ? MemoryAccess::Shared : MemoryAccess::GPUOnly;

    VkMemoryAllocateFlagsInfo memAllocFlags = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
        .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
        .deviceMask = 0,
    };

    VkMemoryAllocateInfo memAlloc = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = &memAllocFlags,
        .allocationSize = desc.size,
        .memoryTypeIndex = findMemoryTypeIndex(device, impl->m_physicalDevice, 0, access),
    };
    VkDeviceMemory mem;
    handleResult(vkAllocateMemory(device, &memAlloc, nullptr, &mem));
    return impl->memoryHeaps.allocate({ mem, desc.size, access });
}

void Device::Destroy(MemoryHeap heap)
{
    if (!impl->memoryHeaps.isValid(heap))
        return;

    auto heapInfo = impl->memoryHeaps.get(heap);
    vkFreeMemory(impl->m_device, heapInfo.memory, nullptr);

    impl->memoryHeaps.free(heap);
}