// standard headers
#include <limits>
#include <memory>
#include <vector>
#include <algorithm>
#include <print>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/Allocators/FrameAllocator.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Core::Allocators;

void FrameAllocator::Create(Device& dev, const FramellocatorDesc& desc)
{
    device = &dev;
}

void FrameAllocator::Destroy()
{

}

auto FrameAllocator::AllocateMemory(uint32_t size) -> MemoryView
{
    return {};
}

auto FrameAllocator::AllocateTexture(VkImage img) -> TextureView
{
    return {};
}

void FrameAllocator::ReleaseMemory(MemoryView view)
{

}

void FrameAllocator::ReleaseTexture(TextureView view)
{

}