/*
Created by @PieroCastillo on 2025-02-13
*/
#ifndef LETTUCE_CORE_ALLOCATORS_HEAP_ALLOCATOR
#define LETTUCE_CORE_ALLOCATORS_HEAP_ALLOCATOR

// project headers
#include "IGPUMemoryResource.hpp"

namespace Lettuce::Core::Allocators
{
    struct HeapllocatorDesc 
    {
        uint32_t blockDefaultSize;
    };

    struct Heap {
        std::vector<MemoryHeap> blockHeaps;
        std::vector<uint64_t> blockOffsets;
        std::vector<uint64_t> blockSizes;
    };

    class HeapAllocator : public IGPUMemoryResource
    {
        Device* device;

        // small buffers
        Heap hSmallBuffers;
        std::vector<MemoryView> mvsSmall;
        // large buffers
        Heap hLargeBuffers;
        std::vector<MemoryView> mvsLarge;
        // small textures
        Heap hSmallTextures;
        std::vector<Texture> tSmall;
        // large textures
        Heap hBigTextures;
        std::vector<Texture> mLarge;

    public:
        void Create(Device&, const HeapllocatorDesc&);
        void Destroy();

        MemoryView AllocateMemory(uint64_t) override;
        Texture AllocateTexture(const TextureDesc&) override;
        void ReleaseMemory(const MemoryView&) override;
        void ReleaseTexture(Texture) override;
    };
};
#endif // LETTUCE_CORE_ALLOCATORS_HEAP_ALLOCATOR