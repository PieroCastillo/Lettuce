// project headers
#include "Lettuce/Core/common.hpp"

void Lettuce::Core::setDebugName(const IDevice& device, VkObjectType type, uint64_t handle, const std::string& name)
{
    if(device.isDebug())
    {
        VkDebugUtilsObjectNameInfoEXT nameInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .objectType = type,
            .objectHandle = handle,
            .pObjectName = name.c_str(),
        };
       handleResult(vkSetDebugUtilsObjectNameEXT(device.m_device, &nameInfo));
    }
}

uint32_t Lettuce::Core::findMemoryTypeIndex(VkDevice device, VkPhysicalDevice gpu, uint32_t typeFilter, MemoryAccess access)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(gpu, &memProperties);
    VkMemoryPropertyFlags props;
    switch (access)
    {
    case MemoryAccess::GPUOnly:
        props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        break;
    case MemoryAccess::Shared:
        props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        break;
    }

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & props) == props) {
            return i;
        }
    }

    throw LettuceException(LettuceResult::RequiredMemoryNotFound);
}