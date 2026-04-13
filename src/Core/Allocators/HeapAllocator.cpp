// standard headers
#include <limits>
#include <memory>
#include <vector>
#include <algorithm>
#include <print>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/Allocators/HeapAllocator.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Core::Allocators;

void HeapAllocator::Create(Device& dev, const HeapllocatorDesc& desc)
{
    device = &dev;
}

void HeapAllocator::Destroy()
{

}

auto HeapAllocator::AllocateMemory(uint32_t size) -> MemoryView
{
    return {};
}

auto HeapAllocator::AllocateTexture(VkImage img) -> TextureView
{
    return {};
}

void HeapAllocator::ReleaseMemory(MemoryView view)
{

}

void HeapAllocator::ReleaseTexture(TextureView view)
{

}