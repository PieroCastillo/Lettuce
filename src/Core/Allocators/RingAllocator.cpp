// standard headers
#include <limits>
#include <memory>
#include <vector>
#include <algorithm>
#include <print>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/Allocators/RingAllocator.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Core::Allocators;

void RingAllocator::Create(Device& dev, const RingllocatorDesc& desc)
{
    device = &dev;
}

void RingAllocator::Destroy()
{

}

auto RingAllocator::AllocateMemory(uint32_t size) -> MemoryView
{
    return {};
}

auto RingAllocator::AllocateTexture(VkImage img) -> TextureView
{
    return {};
}

void RingAllocator::ReleaseMemory(MemoryView view)
{

}

void RingAllocator::ReleaseTexture(TextureView view)
{

}