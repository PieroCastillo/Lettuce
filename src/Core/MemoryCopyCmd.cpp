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

void CommandBuffer::MemoryCopy(const HostToMemoryCopy& desc)
{

}

void CommandBuffer::MemoryCopy(const HostToTextureCopy& desc)
{

}

void CommandBuffer::MemoryCopy(const MemoryToMemoryCopy& desc)
{

}

void CommandBuffer::MemoryCopy(const MemoryToTextureCopy& desc)
{

}

void CommandBuffer::MemoryCopy(const TextureToMemory& desc)
{

}

void CommandBuffer::Fill(MemoryView view, uint32_t value, uint32_t count)
{

}