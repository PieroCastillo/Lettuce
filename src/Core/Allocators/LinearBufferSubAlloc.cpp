// project headers
#include "Lettuce/Core/Allocators/LinearBufferSubAlloc.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Core::Allocators;

// void LinearBufferSubAlloc::Create(const IDevice& device, const LinearBufferSubAllocCreateInfo& createInfo)
// {
//     m_device = device.m_device;
//     m_size = createInfo.maxSize;
//     m_access = createInfo.memoryAccess;
//     m_offset = 0;
//     m_alignment = createInfo.alignment;

//     VkBufferCreateInfo bufferCI = {
//         .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
//         .size = m_size,
//         .usage = mapAllocatorUsageToVk(createInfo.memoryUsage),
//         .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
//     };

//     handleResult(vkCreateBuffer(m_device, &bufferCI, nullptr, &m_buffer));

//     VkMemoryRequirements memReqs;
//     vkGetBufferMemoryRequirements(m_device, m_buffer, &memReqs);

//     VkMemoryAllocateFlagsInfo memoryFlags = {
//         .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
//         .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
//         .deviceMask = 0,
//     };

//     VkMemoryAllocateInfo memoryAI =
//     {
//         .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
//         .pNext = &memoryFlags,
//         .allocationSize = m_size,
//         .memoryTypeIndex = findMemoryTypeIndex(m_device, device.m_physicalDevice, memReqs.memoryTypeBits, createInfo.memoryAccess),
//     };

//     handleResult(vkAllocateMemory(m_device, &memoryAI, nullptr, &m_memory));
//     handleResult(vkBindBufferMemory(m_device, m_buffer, m_memory, 0));

//     if (m_access != MemoryAccess::FastGPUReadWrite)
//     {
//         handleResult(vkMapMemory(m_device, m_memory, 0, m_size, 0, &m_mappedData));
//     }

//     setDebugName(device, VK_OBJECT_TYPE_DEVICE_MEMORY, (uint64_t)m_memory, createInfo.name + "Memory");
//     setDebugName(device, VK_OBJECT_TYPE_BUFFER, (uint64_t)m_buffer, createInfo.name + "Buffer");
// }

// void LinearBufferSubAlloc::Release()
// {
//     if (m_access != MemoryAccess::FastGPUReadWrite)
//     {
//         vkUnmapMemory(m_device, m_memory);
//     }
//     vkDestroyBuffer(m_device, m_buffer, nullptr);
//     vkFreeMemory(m_device, m_memory, nullptr);
// }

// void LinearBufferSubAlloc::Reset()
// {
//     m_offset = 0;
// }

// BufferAllocation LinearBufferSubAlloc::do_balloc(uint32_t size, VkBufferUsageFlags usage)
// {
//     auto oldOffset = m_offset;
//     m_offset += align_up(size + 1, m_alignment);
//     if (m_offset > (m_size + 1))
//     {
//         throw LettuceException(LettuceResult::OutOfHostMemory);
//     }
//     return BufferAllocation{
//         .buffer = m_buffer,
//         .size = size,
//         .offset = m_offset,
//         .data = (uint32_t*)m_mappedData + m_offset,
//         .handle = 0,
//     };
// }

// ImageAllocation LinearBufferSubAlloc::do_ialloc(VkFormat format, VkExtent2D extent, uint32_t mipLevels, uint32_t arrayLayers, VkImageUsageFlags usage)
// {
//     throw LettuceException(LettuceResult::InvalidOperation);
// }

// void LinearBufferSubAlloc::do_bdeallocate(const BufferAllocation& allocation)
// {

// }

// void LinearBufferSubAlloc::do_ideallocate(const ImageAllocation& allocation)
// {
//     throw LettuceException(LettuceResult::InvalidOperation);
// }

// MemoryAccess LinearBufferSubAlloc::GetMemoryAccess()
// {
//     return m_access;
// }