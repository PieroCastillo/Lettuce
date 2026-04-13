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

// TODO: memory

auto CommandBuffer::MemoryCopy(const HostToMemoryCopy& desc)
{

}

auto CommandBuffer::MemoryCopy(const HostToTextureCopy& desc)
{

}

auto CommandBuffer::MemoryCopy(const MemoryToMemoryCopy& desc)
{

}

auto CommandBuffer::MemoryCopy(const MemoryToTextureCopy& desc)
{

}

auto CommandBuffer::MemoryCopy(const TextureToMemory& desc)
{

}

auto CommandBuffer::Fill(MemoryView view, uint32_t value, uint32_t count)
{

}