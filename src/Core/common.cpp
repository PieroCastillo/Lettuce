// project headers
#include "Lettuce/Core/common.hpp"

uint32_t findMemoryTypeIndex(VkDevice device, VkPhysicalDevice gpu, uint32_t typeFilter, MemoryAccess access)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(gpu, &memProperties);
    VkMemoryPropertyFlags props;
    switch (access)
    {
    case MemoryAccess::FastGPUReadWrite:
        props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        break;
    case MemoryAccess::FastCPUWriteGPURead:
        props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        break;
    case MemoryAccess::FastGPUWriteCPURead:
        props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        break;
    case MemoryAccess::FastCPUReadWrite:
        props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        break;
    }

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & props) == props) {
            return i;
        }
    }

    throw LettuceException(LettuceResult::RequiredMemoryNotFound);
}