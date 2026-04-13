/*
Created by @PieroCastillo on 2025-04-13
*/
#ifndef LETTUCE_CORE_ALLOCATORS_FRAME_ALLOCATOR
#define LETTUCE_CORE_ALLOCATORS_FRAME_ALLOCATOR

// project headers
#include "../IGPUMemoryResource.hpp"
#include "common.hpp"

namespace Lettuce::Core::Allocators
{
    struct FrameAllocatorDesc
    {
        bool cpuVisible;
    };

    class FrameAllocator : public IGPUMemoryResource
    {
        DeviceImpl* device;
    public:
        explicit FrameAllocator(Device& dev, const FrameAllocatorDesc& desc) { Create(dev, desc); }

        void Create(Device&, const FrameAllocatorDesc&);
        void Destroy();

        auto AllocateMemory(uint32_t size) -> MemoryView override;
        auto AllocateTexture(VkImage) -> TextureView override;
        void ReleaseMemory(MemoryView) override;
        void ReleaseTexture(TextureView) override;
    };
};
#endif // LETTUCE_CORE_ALLOCATORS_FRAME_ALLOCATOR