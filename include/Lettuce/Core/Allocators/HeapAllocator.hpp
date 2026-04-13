/*
Created by @PieroCastillo on 2025-02-13
*/
#ifndef LETTUCE_CORE_ALLOCATORS_HEAP_ALLOCATOR
#define LETTUCE_CORE_ALLOCATORS_HEAP_ALLOCATOR

// project headers
#include "../IGPUMemoryResource.hpp"
#include "common.hpp"

namespace Lettuce::Core::Allocators
{
    struct HeapAllocatorDesc
    {
        uint32_t blockDefaultSize;
        bool cpuVisible;
    };

    struct Heap {
        std::vector<VkDeviceMemory> blockHeaps;
        std::vector<uint64_t> blockOffsets;
        std::vector<uint64_t> blockSizes;
    };

    class HeapAllocator : public IGPUMemoryResource
    {
        DeviceImpl* device;

        // small buffers
        Heap hSmallBuffers;
        std::vector<MemoryView> mvsSmall;
        // large buffers
        Heap hLargeBuffers;
        std::vector<MemoryView> mvsLarge;
        // small textures
        Heap hSmallTextures;
        std::vector<TextureView> tSmall;
        // large textures
        Heap hBigTextures;
        std::vector<TextureView> mLarge;

    public:
        explicit HeapAllocator(Device& dev, const HeapAllocatorDesc& desc) { Create(dev, desc); }

        void Create(Device&, const HeapAllocatorDesc&);
        void Destroy();

        auto AllocateMemory(uint32_t size) -> MemoryView override;
        auto AllocateTexture(VkImage) -> TextureView override;
        void ReleaseMemory(MemoryView) override;
        void ReleaseTexture(TextureView) override;
    };
};
#endif // LETTUCE_CORE_ALLOCATORS_HEAP_ALLOCATOR