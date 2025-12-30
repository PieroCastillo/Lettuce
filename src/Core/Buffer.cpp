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

Buffer Device::CreateBuffer(const BufferDesc& desc, const MemoryBindDesc& bindDesc)
{
    VkDeviceMemory mem = impl->memoryHeaps.get(bindDesc.heap).memory;

    VkBufferCreateInfo bufferCI = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = desc.size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    VkBuffer buffer;
    handleResult(vkCreateBuffer(impl->m_device, &bufferCI, nullptr, &buffer));
    handleResult(vkBindBufferMemory(impl->m_device, buffer, mem, bindDesc.heapOffset));

    return impl->buffers.allocate({ buffer, mem, desc.size, bindDesc.heapOffset });
}

void Device::Destroy(Buffer buffer)
{
    vkDestroyBuffer(impl->m_device, impl->buffers.get(buffer).buffer, nullptr);
    impl->buffers.free(buffer);
}