// standard headers
#include <limits>
#include <memory>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/TableGroup.hpp"

using namespace Lettuce::Core;

void TableGroup::Create(const IDevice& device, const TableGroupCreateInfo& createInfo)
{
    if (device.m_device == VK_NULL_HANDLE) {
        throw LettuceException(LettuceResult::InvalidDevice);
    }

    m_device = device.m_device;
    m_transferQueue = device.m_transferQueue;

    // setup memory & buffer
    VkMemoryAllocateInfo memAI = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = 1024 * 1024, // TODO: calculate size
        .memoryTypeIndex = 0, // TODO: find proper memory type index
    };
    handleResult(vkAllocateMemory(m_device, &memAI, nullptr, &m_memory));

    VkBufferCreateInfo bufferCI = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = 1024 * 1024, // TODO: calculate size
        .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    // setup command buffer & pool
    VkCommandPoolCreateInfo cmdPoolCI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = device.m_transferQueueFamilyIndex,
    };

    handleResult(vkCreateCommandPool(m_device, &cmdPoolCI, nullptr, &m_cmdPool));

    VkCommandBufferAllocateInfo cmdAllocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = m_cmdPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    handleResult(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &m_cmd));
}

void  TableGroup::Release() 
{
    // TODO: wait for copies complete
    vkDestroyBuffer(m_device, m_buffer, nullptr);
    vkFreeMemory(m_device, m_memory, nullptr);
    vkFreeCommandBuffers(m_device, m_cmdPool, 1, &m_cmd);
    vkDestroyCommandPool(m_device, m_cmdPool, nullptr);

    m_tables.clear();
    m_tableOffsets.clear();
}

void  TableGroup::FlushCopies() 
{

}