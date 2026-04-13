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

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Core/common.hpp"

using namespace Lettuce::Core;

// TODO: memory

auto Device::CreateMemoryView(const MemoryViewDesc& desc)
{
    ////////////////////////////////////////////////
    auto memInfo = impl->memoryHeaps.get(bindDesc.heap);

    VkBufferCreateInfo bufferCI = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = desc.size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    VkBuffer buffer;
    handleResult(vkCreateBuffer(impl->m_device, &bufferCI, nullptr, &buffer));

    handleResult(vkBindBufferMemory(impl->m_device, buffer, memInfo.memory, bindDesc.heapOffset));

    VkBufferDeviceAddressInfo addressInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = buffer,
    };

    switch(desc.policy)
    {
        case AllocationPolicy::Frame: 
            break;
    }

    ////////////////////////////////////////////////

    void* cpuAddress = memInfo.access == MemoryAccess::Shared ? ((uint64_t*)memInfo.baseCpuAddress + bindDesc.heapOffset) : nullptr;
    uint64_t gpuAddress = vkGetBufferDeviceAddress(impl->m_device, &addressInfo);

    return impl->buffers.allocate({ buffer, memInfo.memory, desc.size, bindDesc.heapOffset, cpuAddress, gpuAddress });
}

auto Device::Destroy(MemoryView view)
{
    /*
    auto info = impl->buffers.get(buffer);

    vkDestroyBuffer(impl->m_device, impl->buffers.get(buffer).buffer, nullptr);
    impl->buffers.free(buffer);
    */
}