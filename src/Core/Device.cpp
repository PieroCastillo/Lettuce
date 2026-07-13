// standard headers
#include <memory>
#include <array>
#include <print>

// project headers
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Core/common.hpp"

using namespace Lettuce::Core;

Device::Device(const DeviceDesc& desc)
{
    Create(desc);
}

Device::~Device()
{
    Destroy();
}

Device::Device(Device&& other) noexcept : impl(std::exchange(other.impl, nullptr))
{
}

Device& Device::operator=(Device&& other) noexcept
{
    if (this != &other)
    {
        Destroy();
        impl = std::exchange(other.impl, nullptr);
    }

    return *this;
}

void Device::Create(const DeviceDesc& desc)
{
    if (impl)
        throw std::logic_error("Device::Create cannot be called from initizalized Device.");

    auto nimpl = new DeviceImpl;

    try
    {
        nimpl->Create(desc);
    }
    catch (...)
    {
        delete nimpl;
        throw;
    }

    impl = nimpl;
}

void Device::Destroy() noexcept
{
    if (!impl)
        return;

    impl->Release();
    delete impl;
    impl = nullptr;
}

void Device::WaitFor(QueueType queueType)
{
    VkQueue queue;
    switch (queueType)
    {
    case QueueType::Graphics: queue = impl->m_graphicsQueue; break;
    case QueueType::Compute: queue = impl->m_computeQueue; break;
    case QueueType::Copy: queue = impl->m_transferQueue; break;
    }
    handleResult(vkQueueWaitIdle(queue));
}

auto Device::SupportMeshShader() -> bool
{
    return impl->features.MeshShading;
}
auto Device::SupportNeuralShading() -> bool
{
    return impl->features.NeuralShading;
}
auto Device::SupportNeuralShadingNV() -> bool
{
    return impl->features.NeuralShadingNV;
}
auto Device::SupportRayTracing() -> bool
{
    return impl->features.RayTracing;
}
auto Device::SupportRayTracingNV() -> bool
{
    return impl->features.RayTracingNV;
}
auto Device::SupportFragmentShadingRate() -> bool
{
    return impl->features.FragmentShadingRate;
}
auto Device::QueryPreferredThreadCount() -> uint32_t
{
    return impl->props.preferredThreadCount;
}