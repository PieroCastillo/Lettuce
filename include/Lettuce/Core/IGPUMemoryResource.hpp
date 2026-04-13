/*
Created by @PieroCastillo on 2025-11-4
*/
#ifndef LETTUCE_CORE_IGPUMEMORYRESOURCE
#define LETTUCE_CORE_IGPUMEMORYRESOURCE

// project headers
#include "api.hpp"

namespace Lettuce::Core
{
    /*
    DeviceImpl selects if its host/device and by policy
    so, for raw memory, only size is needed
    */
    class IGPUMemoryResource
    {
    public:
        virtual auto AllocateMemory(uint32_t size) -> MemoryView = 0;
        virtual auto AllocateTexture(VkImage) -> TextureView = 0;
        virtual void ReleaseMemory(MemoryView) = 0;
        virtual void ReleaseTexture(TextureView) = 0;
    };
};
#endif // LETTUCE_CORE_IGPUMEMORYRESOURCE