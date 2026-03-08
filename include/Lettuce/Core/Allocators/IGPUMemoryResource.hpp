/*
Created by @PieroCastillo on 2025-11-4
*/
#ifndef LETTUCE_CORE_ALLOCATORS_IGPUMEMORYRESOURCE
#define LETTUCE_CORE_ALLOCATORS_IGPUMEMORYRESOURCE

// project headers
#include "../api.hpp"

namespace Lettuce::Core::Allocators
{
    class IGPUMemoryResource
    {
    public:
        virtual auto AllocateMemory(uint64_t size) -> MemoryView = 0;
        virtual auto AllocateTexture(const TextureDesc&) -> Texture = 0;
        virtual auto AllocateRenderTarget(const RenderTargetDesc&) -> RenderTarget = 0;
        virtual void ReleaseMemory(const MemoryView&) = 0;
        virtual void ReleaseTexture(Texture) = 0;
        virtual void ReleaseRenderTarget(RenderTarget) = 0;
    };
};
#endif // LETTUCE_CORE_ALLOCATORS_IGPUMEMORYRESOURCE