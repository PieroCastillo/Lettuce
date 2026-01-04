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
        virtual MemoryView AllocateMemory(uint64_t size) = 0;
        virtual Texture AllocateTexture(const TextureDesc&) = 0;
        virtual void ReleaseMemory(const MemoryView&) = 0;
        virtual void ReleaseTexture(Texture) = 0;
    };
};
#endif // LETTUCE_CORE_ALLOCATORS_IGPUMEMORYRESOURCE