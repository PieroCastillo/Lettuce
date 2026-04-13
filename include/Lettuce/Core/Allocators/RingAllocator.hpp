/*
Created by @PieroCastillo on 2025-04-13
*/
#ifndef LETTUCE_CORE_ALLOCATORS_RING_ALLOCATOR
#define LETTUCE_CORE_ALLOCATORS_RING_ALLOCATOR

// project headers
#include "../IGPUMemoryResource.hpp"
#include "common.hpp"

namespace Lettuce::Core::Allocators
{
    struct RingAllocatorDesc
    {
        bool cpuVisible;
    };

    class RingAllocator : public IGPUMemoryResource
    {
        DeviceImpl* device;
    public:
        explicit RingAllocator(Device& dev, const RingAllocatorDesc& desc) { Create(dev, desc); }

        void Create(Device&, const RingAllocatorDesc&);
        void Destroy();

        auto AllocateMemory(uint32_t size) -> MemoryView override;
        auto AllocateTexture(VkImage) -> TextureView override;
        void ReleaseMemory(MemoryView) override;
        void ReleaseTexture(TextureView) override;
    };
};
#endif // LETTUCE_CORE_ALLOCATORS_RING_ALLOCATOR