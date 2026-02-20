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

void Allocators::HeapAllocator::Create(Device& dev, const Allocators::HeapllocatorDesc& desc)
{
    device = &dev;
}

void Allocators::HeapAllocator::Destroy()
{
    
}

MemoryView Allocators::HeapAllocator::AllocateMemory(uint64_t size)
{
    return {};
}

Texture Allocators::HeapAllocator::AllocateTexture(const TextureDesc& desc)
{
    return {};
}

void Allocators::HeapAllocator::ReleaseMemory(const MemoryView& view)
{

}

void Allocators::HeapAllocator::ReleaseTexture(Texture tex)
{

}