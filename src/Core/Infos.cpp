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

BufferInfo Device::GetBufferInfo(Buffer buffer) const
{
    auto res = impl->buffers.get(buffer);
    return { res.size, res.cpuAddress, res.gpuAddress };
}

ResourceInfo Device::GetResourceInfo(Texture texture) const
{
    auto res = impl->textures.get(texture);
    return { res.size, res.width, res.height };
}

ResourceInfo Device::GetResourceInfo(RenderTarget renderTarget) const
{
    auto res = impl->renderTargets.get(renderTarget);
    return { res.size, res.width, res.height };
}