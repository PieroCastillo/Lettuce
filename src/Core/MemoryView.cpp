// standard headers
#include <array>
#include <memory>
#include <print>
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <ranges>

// external headers
#include <volk.h>
#include <vk_mem_alloc.h>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Core/common.hpp"

using namespace Lettuce::Core;

auto Device::CreateMemoryView(const MemoryViewDesc& desc) -> MemoryView
{
    VkBuffer buffer;
    VmaAllocation alloc;
    VmaAllocationInfo allocI;

    VkBufferCreateInfo bufferCI = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = desc.size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT ,
    };
    VmaAllocationCreateInfo allocCI = {
        .flags = desc.cpuVisible ? VMA_ALLOCATION_CREATE_MAPPED_BIT : (VmaAllocationCreateFlags)0,
        .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | (desc.cpuVisible ? VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : (VkMemoryPropertyFlags)0),
    };
    handleResult(vmaCreateBuffer(impl->m_allocator, &bufferCI, &allocCI, &buffer, &alloc, &allocI));

    VkBufferDeviceAddressInfo bdaInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = buffer,
    };
    auto devAddr = vkGetBufferDeviceAddress(impl->m_device, &bdaInfo);

    uint8_t* buffCPUAddr = nullptr;
    if(desc.cpuVisible)
        vmaMapMemory(impl->m_allocator, alloc, (void**)(&buffCPUAddr));

    return impl->memories.allocate({buffer, allocI.deviceMemory, allocI.size, allocI.offset, buffCPUAddr, devAddr, alloc});
}

void Device::Destroy(MemoryView view)
{
    auto info = impl->memories.get(view);

    if(info.isView)
        return;

    if(info.cpuAddress)
        vmaUnmapMemory(impl->m_allocator, info.allocation);

    vmaDestroyBuffer(impl->m_allocator, info.buffer, info.allocation);
    impl->memories.free(view);
}