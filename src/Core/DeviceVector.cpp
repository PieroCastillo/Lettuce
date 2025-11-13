// project headers
#include "Lettuce/Core/DeviceVector.hpp"

using namespace Lettuce::Core;

void DeviceVectorBase::Create(const IDevice& device, const DeviceVectorCreateInfo& createInfo, uint32_t elementSize)
{
    m_device = device.m_device;
    m_allocator = createInfo.allocator;
    m_elementSize = elementSize;
    m_maxSize = createInfo.maxCount * elementSize;
    m_offset = 0;

    m_allocation = m_allocator->do_balloc(m_maxSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
        VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    if (m_allocator->GetMemoryAccess() == MemoryAccess::FastGPUReadWrite)
    {
        // TODO: impl
    }
}

void DeviceVectorBase::Release()
{
    if (m_allocator->GetMemoryAccess() == MemoryAccess::FastGPUReadWrite)
    {
        // TODO: impl
    }

    m_allocator->do_bdeallocate(m_allocation);
}

void DeviceVectorBase::PushRange(void* src, uint32_t count)
{
    if (m_allocator->GetMemoryAccess() == MemoryAccess::FastGPUReadWrite)
    {
        // TODO: impl
        return;
    }

    uint32_t newOffset = m_offset + (m_elementSize * count);
    if (newOffset >= m_maxSize)
    {
        throw LettuceException(LettuceResult::OutOfDeviceMemory);
    }

    memcpy((uint64_t*)(m_allocation.data) + m_offset, src, m_elementSize * count);
    m_offset = newOffset;
}

void DeviceVectorBase::Flush()
{
    if (m_allocator->GetMemoryAccess() == MemoryAccess::FastGPUReadWrite)
    {
        // TODO: impl
    }
}

void DeviceVectorBase::Reset()
{
    m_offset = 0;
}