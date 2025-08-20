// standard headers

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/Buffer.hpp"

using namespace Lettuce::Core;

LettuceResult Buffer::Create(VkDevice device, const BufferCreateInfo& createInfo)
{
    m_device = device;
    m_size = createInfo.size;

    VkBufferCreateInfo bufferCI = {
        .sType =    VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .flags = VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT,
        .size = createInfo.size,
        .usage = createInfo.usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    handleResult(vkCreateBuffer(device, &bufferCI, nullptr, &m_buffer), result);

    VkBufferDeviceAddressInfo bufferDAI = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = m_buffer,
    };
    m_address = vkGetBufferDeviceAddress(m_device, &bufferDAI);
    
    return LettuceResult::Success;
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