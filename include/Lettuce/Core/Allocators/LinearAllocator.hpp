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
        uint64_t bufferSize;
        uint64_t imageSize;
        bool cpuVisible;
    };

    class LinearAllocator : public IGPUMemoryResource
    {
        Device device;
        MemoryHeap memory;
        Buffer buffer;
        std::vector<Texture> textures;
        uint64_t memoryHeapSize;
        uint64_t texturesMemoryOffset;
        uint64_t* bufferCPUAddress;
        uint64_t bufferGPUAddress;

        uint64_t currentTextureOffset;
        uint64_t currentBufferOffset;

        uint64_t* currentBufferCPUAddress;
        uint64_t currentBufferGPUAddress;
    public:
        void Create(Device, const LinearAllocatorDesc&);
        void Destroy();

        MemoryView AllocateMemory(uint64_t);
        Texture AllocateTexture(const TextureDesc&);
        void ReleaseMemory(const MemoryView&);
        void ReleaseTexture(Texture);
        void ResetMemory();
    };
};
#endif // LETTUCE_CORE_ALLOCATORS_LINEAR_ALLOCATOR