// standard headers
#include <memory>
#include <array>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/Memory.hpp"

using namespace Lettuce::Core;

LettuceResult Memory::Create(const std::weak_ptr<IDevice>& device, const MemoryCreateInfo& createInfo)
{
    m_device = device->m_device;

    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

    constexpr std::array<VkMemoryPropertyFlags, 3> memoryPropFlagsTable = {
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
    };
    constexpr VkMemoryPropertyFlags memoryFlags = memoryPropFlagsTable[(int)createInfo.access];
    uint32_t memTypeIdx = 0;

    for (uint32_t j = 0; j < memProps.memoryTypeCount; ++j)
    {
        if (memProps.memoryTypes[j] == memoryFlags)
        {
            memTypeIdx = j;
        }
    }

    VkMemoryAllocateInfo memoryAI =
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = createInfo.size,
        .memoryTypeIndex = memTypeIdx,
    };
    checkResult(vkAllocateMemory(m_device, &memoryAI, nullptr, &m_memory));

    
    return LettuceResult::Success;
}

void Memory::Release()
{
    vkFreeMemory(m_device, m_memory, nullptr);
}