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

void Device::MemoryCopy(const HostToMemoryCopy& desc)
{
    throw LettuceException(LettuceResult::NotImplemented);
}

void Device::MemoryCopy(const HostToTextureCopy& desc)
{
    throw LettuceException(LettuceResult::NotImplemented);
}

void Device::MemoryCopy(const MemoryToMemoryCopy& desc)
{
    throw LettuceException(LettuceResult::NotImplemented);
}

void Device::MemoryCopy(const MemoryToTextureCopy& desc)
{
    throw LettuceException(LettuceResult::NotImplemented);
}