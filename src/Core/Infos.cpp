// standard headers
#include <limits>
#include <memory>
#include <vector>
#include <algorithm>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Core/common.hpp"

using namespace Lettuce::Core;

auto Device::GetMemoryViewInfo(MemoryView view) const -> MemoryViewInfo
{
    // auto res = impl->buffers.get(buffer);
    // return { res.size, res.cpuAddress, res.gpuAddress, res.memoryOffset };
    return {};
}

auto Device::GetResourceInfo(TextureView view) const -> TextureViewInfo
{
    // auto res = impl->textures.get(texture);
    // return { res.size, res.width, res.height, res.memoryOffset };
    return {};
}