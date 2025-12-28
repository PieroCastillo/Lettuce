// project headers
#include "Lettuce/Core/Allocators/LinearImageAlloc.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Core::Allocators;

// void LinearImageAlloc::Create(const IDevice& device, const LinearImageAllocCreateInfo& createInfo)
// {
//     m_device = device.m_device;

//     VkMemoryAllocateInfo memoryAI =
//     {
//         .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
//         .allocationSize = m_size,
//         .memoryTypeIndex = findMemoryTypeIndex(m_device, device.m_physicalDevice, 0, createInfo.memoryAccess),
//     };

//     handleResult(vkAllocateMemory(m_device, &memoryAI, nullptr, &m_memory));
//     m_offset = 0;
// }

// void LinearImageAlloc::Release()
// {
//     Reset();
//     vkFreeMemory(m_device, m_memory, nullptr);
// }

// void LinearImageAlloc::Reset()
// {
//     for(const auto& img : m_images)
//     {
//         vkDestroyImage(m_device, img, nullptr);
//     }
//     m_images.clear();
//     m_images.shrink_to_fit();
//     m_offset = 0;
// }

// BufferAllocation LinearImageAlloc::do_balloc(uint32_t size, VkBufferUsageFlags usage)
// {
//     throw LettuceException(LettuceResult::InvalidOperation);
// }

// ImageAllocation LinearImageAlloc::do_ialloc(VkFormat format, VkExtent2D extent, uint32_t mipLevels, uint32_t arrayLayers, VkImageUsageFlags usage)
// { 
//     VkImageCreateInfo imgCI = {
//         .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
//         .imageType = VK_IMAGE_TYPE_2D,
//         .format = format,
//         .extent = {extent.width, extent.height, 1},
//         .mipLevels = mipLevels,
//         .arrayLayers = arrayLayers,
//         .samples = VK_SAMPLE_COUNT_1_BIT,
//         .tiling = VK_IMAGE_TILING_OPTIMAL,
//         .usage = usage,
//         .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
//         .initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED,
//     };
//     VkImage image;
//     handleResult(vkCreateImage(m_device, &imgCI, nullptr, &image));

//     // bind to memory
//     VkMemoryRequirements reqs;
//     vkGetImageMemoryRequirements(m_device, image, &reqs);
    
//     m_offset = align_up(m_offset, reqs.alignment);

//     if(m_offset + reqs.size > m_size)
//     {
//         throw LettuceResult(LettuceResult::OutOfDeviceMemory);
//     }

//     handleResult(vkBindImageMemory(m_device, image, m_memory, m_offset));
//     m_offset += reqs.size;

//     m_images.push_back(image);

//     ImageAllocation alloc = {
//         .image = image,
//         .handle = 0,
//     };
//     return alloc;
// }

// void LinearImageAlloc::do_bdeallocate(const BufferAllocation& allocation)
// {
//     throw LettuceException(LettuceResult::InvalidOperation);
// }

// void LinearImageAlloc::do_ideallocate(const ImageAllocation& allocation)
// {
    
// }

// MemoryAccess LinearImageAlloc::GetMemoryAccess()
// {
//     return m_access;
// }