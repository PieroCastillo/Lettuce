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
    impl->Create({ desc.preferDedicated });
}

void Device::Destroy()
{
    impl->Release();
}