/*
Created by @PieroCastillo on 2025-12-30
*/
#ifndef LETTUCE_CORE_ALLOCATORS_LINEAR_ALLOCATOR
#define LETTUCE_CORE_ALLOCATORS_LINEAR_ALLOCATOR

// project headers
#include "IGPUMemoryResource.hpp"

namespace Lettuce::Core::Allocators
{
    struct LinearAllocatorDesc
    {
        uint64_t maxBufferMemorySize;
        uint64_t maxImageMemorySize;
        uint64_t maxRenderTargetsMemorySize;
        bool cpuVisible;
    };

    class LinearAllocator : public IGPUMemoryResource
    {
        Device* device;
        MemoryHeap memory;
        Buffer buffer;
        std::vector<Texture> textures;
        std::vector<RenderTarget> renderTargets;
        uint64_t memoryHeapSize;

        uint64_t texturesMemoryOffset;
        uint64_t renderTargetsMemoryOffset;
        
        HostAddress bufferCPUAddress;
        uint64_t bufferGPUAddress;

        uint64_t currentRenderTargetOffset;
        uint64_t currentTextureOffset;
        uint64_t currentBufferOffset;

        HostAddress currentBufferCPUAddress;
        uint64_t currentBufferGPUAddress;
        uint64_t currentBufferUsage;
    public:
        void Create(Device&, const LinearAllocatorDesc&);
        void Destroy();

        auto AllocateMemory(uint64_t) -> MemoryView;
        auto AllocateTexture(const TextureDesc&) -> Texture;
        auto AllocateRenderTarget(const RenderTargetDesc&) -> RenderTarget;
        void ReleaseMemory(const MemoryView&);
        void ReleaseTexture(Texture);
        void ReleaseRenderTarget(RenderTarget);
        void ResetMemory();
    };
};
#endif // LETTUCE_CORE_ALLOCATORS_LINEAR_ALLOCATOR