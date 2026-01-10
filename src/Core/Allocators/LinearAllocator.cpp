// standard headers
#include <limits>
#include <memory>
#include <vector>
#include <algorithm>
#include <print>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/Allocators/LinearAllocator.hpp"

using namespace Lettuce::Core;

void Allocators::LinearAllocator::Create(Device dev, const LinearAllocatorDesc& desc)
{
    device = dev;
    MemoryHeapDesc heapDesc = {
        .size = (desc.bufferSize + desc.imageSize),
        .cpuVisible = desc.cpuVisible,
    };
    memory = device.CreateMemoryHeap(heapDesc);

    BufferDesc bufferDesc = {
        .size = desc.bufferSize,
    };
    MemoryBindDesc bindDesc = {
        .heap = memory,
        .heapOffset = 0,
    };
    buffer = device.CreateBuffer(bufferDesc, bindDesc);

    texturesMemoryOffset = desc.bufferSize + 1;

    currentBufferOffset = 0;
    currentTextureOffset = texturesMemoryOffset;
    memoryHeapSize = desc.bufferSize + desc.imageSize;

    auto bufferInfo = device.GetBufferInfo(buffer);
    bufferCPUAddress = (uint64_t*)bufferInfo.cpuAddress;
    bufferGPUAddress = bufferInfo.gpuAddress;

    currentBufferCPUAddress = (uint64_t*)bufferInfo.cpuAddress;
    currentBufferGPUAddress = bufferInfo.gpuAddress;
}

void Allocators::LinearAllocator::Destroy()
{
    device.Destroy(buffer);
    for (auto tex : textures)
    {
        device.Destroy(tex);
    }
    device.Destroy(memory);
    textures.clear();
}

MemoryView Allocators::LinearAllocator::AllocateMemory(uint64_t size)
{
    if ((currentBufferOffset + size) > texturesMemoryOffset)
    {
        return {};
    }

    MemoryView mv = { size, currentBufferCPUAddress, currentBufferGPUAddress, buffer, currentBufferOffset };

    currentBufferCPUAddress += ((currentBufferCPUAddress != nullptr) ? size : 0);
    currentBufferGPUAddress += size;

    return mv;
}

Texture Allocators::LinearAllocator::AllocateTexture(const TextureDesc& desc)
{
    MemoryBindDesc bindDesc = {
        .heap = memory,
        .heapOffset = currentTextureOffset,
    };
    auto tex = device.CreateTexture(desc, bindDesc);
    DebugPrint("[LINEAR ALLOCATOR]", "Texture allocated successfully");
    auto texInfo = device.GetResourceInfo(tex);
    auto newOffset = currentTextureOffset + texInfo.size;

    if (newOffset > memoryHeapSize)
    {
        // TODO: error checking
        DebugPrint("[LINEAR ALLOCATOR]", "Error Texture");
        return {};
    }

    currentTextureOffset = newOffset;
    return tex;
}

void Allocators::LinearAllocator::ReleaseMemory(const MemoryView& view)
{
    // NO OP
}

void Allocators::LinearAllocator::ReleaseTexture(Texture texture)
{
    // NO OP
}

void Allocators::LinearAllocator::ResetMemory()
{
    currentBufferOffset = 0;
    currentBufferCPUAddress = bufferCPUAddress;
    currentBufferGPUAddress = bufferGPUAddress;
}