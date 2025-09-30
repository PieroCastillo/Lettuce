// standard headers
#include <memory>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/Buffer.hpp"

using namespace Lettuce::Core;

void Buffer::Create(const std::weak_ptr<IDevice>& device, const BufferCreateInfo& createInfo)
{
    m_device = (device.lock())->m_device;
    m_size = createInfo.size;

    VkBufferCreateInfo bufferCI = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .flags = VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT,
        .size = createInfo.size,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    if ((device.lock())->supportBufferUsage2) {
        VkBufferUsageFlags2CreateInfo usageFlags2CI = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO,
            .usage = createInfo.usage,
        };
        bufferCI.pNext = &usageFlags2CI;
    }
    else {
        bufferCI.usage = static_cast<VkBufferUsageFlags>(createInfo.usage);
    }

    handleResult(vkCreateBuffer(m_device, &bufferCI, nullptr, &m_buffer));

    VkBufferDeviceAddressInfo bufferDAI = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = m_buffer,
    };
    m_address = vkGetBufferDeviceAddress(m_device, &bufferDAI);
}

void Buffer::Release()
{
    vkDestroyBuffer(m_device, m_buffer, nullptr);
}

uint64_t Buffer::GetAddress()
{
    return m_address;
}

uint32_t Buffer::GetSize()
{
    return m_size;
}