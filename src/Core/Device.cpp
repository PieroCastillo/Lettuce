// standard headers
#include <memory>
#include <array>
#include <print>

// project headers
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Core/common.hpp"

using namespace Lettuce::Core;

void Device::Create(const DeviceDesc& desc)
{
    impl = new DeviceImpl;
    impl->Create({ desc.preferDedicated });
}

void Device::Destroy()
{
    impl->Release();
    delete impl;
}

void Device::WaitFor(QueueType queueType)
{
    VkQueue queue;
    switch (queueType)
    {
    case QueueType::Graphics: queue = impl->m_graphicsQueue; break;
    case QueueType::Compute: queue = impl->m_computeQueue;break;
    case QueueType::Copy: queue = impl->m_transferQueue; break;
    }
    handleResult(vkQueueWaitIdle(queue));
}