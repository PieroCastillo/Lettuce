// standard headers
#include <limits>
#include <memory>
#include <vector>
#include <algorithm>
#include <print>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/common.hpp"
#include "Lettuce/Core/Allocators/LinearAllocator.hpp"

using namespace Lettuce::Core;

void Allocators::LinearAllocator::Create(Device& dev, const LinearAllocatorDesc& desc)
{
    device = &dev;
    auto alignment = 64;

    // SETUP OFFSETS
    texturesMemoryOffset = align_up(desc.maxBufferMemorySize + 1, alignment);
    renderTargetsMemoryOffset = align_up(texturesMemoryOffset + desc.maxImageMemorySize + 1, alignment);

    currentTextureOffset = texturesMemoryOffset;
    currentRenderTargetOffset = renderTargetsMemoryOffset;

    currentBufferOffset = 0;
    memoryHeapSize = renderTargetsMemoryOffset + desc.maxRenderTargetsMemorySize;

    // SETUP BUFFER & HEAP
    MemoryHeapDesc heapDesc = {
        .size = memoryHeapSize,
        .cpuVisible = desc.cpuVisible,
    };
    memory = device->CreateMemoryHeap(heapDesc);

    BufferDesc bufferDesc = {
        .size = desc.maxBufferMemorySize,
    };
    MemoryBindDesc bindDesc = {
        .heap = memory,
        .heapOffset = 0,
    };
    buffer = device->CreateBuffer(bufferDesc, bindDesc);

    // SETUP BUFFER ADDRESSES
    auto bufferInfo = device->GetBufferInfo(buffer);
    bufferCPUAddress = (HostAddress)bufferInfo.cpuAddress;
    bufferGPUAddress = bufferInfo.gpuAddress;

    currentBufferCPUAddress = (HostAddress)bufferInfo.cpuAddress;
    currentBufferGPUAddress = bufferInfo.gpuAddress;

    DebugPrint("[LINEAR ALLOCATOR]", "buffer  memory size: {}", texturesMemoryOffset - 1);
    DebugPrint("                  ", "images  memory size: {}", renderTargetsMemoryOffset - texturesMemoryOffset + 1);
    DebugPrint("                  ", "targets memory size: {}", memoryHeapSize - renderTargetsMemoryOffset + 1);

    currentBufferUsage = 0;
}

void Allocators::LinearAllocator::Destroy()
{
    device->Destroy(buffer);
    for (auto tex : textures)
    {
        device->Destroy(tex);
    }
    for (auto rt : renderTargets)
    {
        device->Destroy(rt);
    }
    device->Destroy(memory);
    textures.clear();
}

MemoryView Allocators::LinearAllocator::AllocateMemory(uint64_t size)
{
    uint64_t alignment = 64;
    uint64_t alignedOffset = (currentBufferOffset + alignment - 1) & ~(alignment - 1);

    if ((alignedOffset + size) > texturesMemoryOffset)
    {
        return {};
    }

    currentBufferOffset = alignedOffset;

    HostAddress cpuAddr = (currentBufferCPUAddress != nullptr) ? (currentBufferCPUAddress + currentBufferOffset) : nullptr;
    uint64_t gpuAddr = currentBufferGPUAddress + currentBufferOffset;

    MemoryView mv = { size, cpuAddr, gpuAddr, buffer, currentBufferOffset };

    currentBufferOffset += size;
    currentBufferUsage = currentBufferOffset;

    return mv;
}

Texture Allocators::LinearAllocator::AllocateTexture(const TextureDesc& desc)
{
    auto alignment = 64;
    MemoryBindDesc bindDesc = {
        .heap = memory,
        .heapOffset = currentTextureOffset,
    };
    auto tex = device->CreateTexture(desc, bindDesc);
    auto texInfo = device->GetResourceInfo(tex);
    auto newOffset = align_up(currentTextureOffset + texInfo.size, alignment);

    if (newOffset >= renderTargetsMemoryOffset)
    {
        // TODO: error checking
        DebugPrint("[LINEAR ALLOCATOR]", "Error Texture");
        device->Destroy(tex);
        return {};
    }

    DebugPrint("[LINEAR ALLOCATOR]", "Texture allocated successfully");
    currentTextureOffset = newOffset;
    return tex;
}

auto Allocators::LinearAllocator::AllocateRenderTarget(const RenderTargetDesc& desc) -> RenderTarget
{
    auto alignment = 64;
    MemoryBindDesc bindDesc = {
        .heap = memory,
        .heapOffset = currentRenderTargetOffset,
    };
    auto rt = device->CreateRenderTarget(desc, bindDesc);
    auto rtInfo = device->GetResourceInfo(rt);
    auto newOffset = align_up(currentRenderTargetOffset + rtInfo.size, alignment);

    if (newOffset >= memoryHeapSize)
    {
        // TODO: error checking
        DebugPrint("[LINEAR ALLOCATOR]", "Error Render Target");
        device->Destroy(rt);
        return {};
    }

    DebugPrint("[LINEAR ALLOCATOR]", "Render Target allocated successfully");
    currentRenderTargetOffset = newOffset;
    return rt;
}

void Allocators::LinearAllocator::ReleaseMemory(const MemoryView& view)
{
    // NO OP
}

void Allocators::LinearAllocator::ReleaseTexture(Texture texture)
{
    // NO OP
}

void Allocators::LinearAllocator::ReleaseRenderTarget(RenderTarget renderTarget)
{
    // no op
}

void Allocators::LinearAllocator::ResetMemory()
{
    currentBufferOffset = 0;
    currentBufferCPUAddress = bufferCPUAddress;
    currentBufferGPUAddress = bufferGPUAddress;
    currentTextureOffset = texturesMemoryOffset;
    currentRenderTargetOffset = renderTargetsMemoryOffset;

    for (auto tex : textures)
    {
        device->Destroy(tex);
    }

    for (auto rt : renderTargets)
    {
        device->Destroy(rt);
    }
}