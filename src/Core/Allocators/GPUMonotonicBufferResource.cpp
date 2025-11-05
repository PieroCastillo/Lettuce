// project headers
#include "Lettuce/Core/Allocators/GPUMonotonicBufferResource.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Core::Allocators;

GPUMonotonicBufferResource::GPUMonotonicBufferResource(const IDevice& device, const GPUMonotonicBufferResourceCreateInfo& createInfo)
{
    m_device = device.m_device;
    m_size = createInfo.maxSize;
    m_access = createInfo.memoryAccess;
    m_offset = 0;

    VkBufferCreateInfo bufferCI = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = m_size,
        .usage = mapAllocatorUsageToVk(createInfo.memoryUsage),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    handleResult(vkCreateBuffer(m_device, &bufferCI, nullptr, &m_buffer));

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(m_device, m_buffer, &memReqs);

    VkMemoryAllocateInfo memoryAI =
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = m_size,
        .memoryTypeIndex = findMemoryTypeIndex(m_device, device.m_physicalDevice, memReqs.memoryTypeBits, createInfo.memoryAccess),
    };

    handleResult(vkAllocateMemory(m_device, &memoryAI, nullptr, &m_memory));
    handleResult(vkBindBufferMemory(m_device, m_buffer, m_memory, 0));

    if (m_access != MemoryAccess::FastGPUReadWrite)
    {
        handleResult(vkMapMemory(m_device, m_memory, 0, m_size, 0, &m_mappedData));
        return;
    }
    throw LettuceException(LettuceResult::NotReady);
}

GPUMonotonicBufferResource::~GPUMonotonicBufferResource()
{
    if (m_access != MemoryAccess::FastGPUReadWrite)
    {
        vkUnmapMemory(m_device, m_memory);
    }
    vkDestroyBuffer(m_device, m_buffer, nullptr);
    vkFreeMemory(m_device, m_memory, nullptr);
}

void GPUMonotonicBufferResource::Reset()
{
    m_offset = 0;
}

void* GPUMonotonicBufferResource::do_allocate(size_t bytes, size_t alignment)
{
    if (m_offset + bytes > m_size) [[unlikely]]
    {
        throw LettuceException(LettuceResult::OutOfHostMemory);
    }    
    void* ptr = (char*)m_mappedData + m_offset;
    m_offset += bytes + 1;

    return ptr;
}

void GPUMonotonicBufferResource::do_deallocate(void*, size_t, size_t)
{
    throw LettuceException(LettuceResult::InvalidOperation);
}

bool GPUMonotonicBufferResource::do_is_equal(const std::pmr::memory_resource& other) const noexcept
{
    return true;
}