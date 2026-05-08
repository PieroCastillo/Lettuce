/*
Created by @PieroCastillo on 2025-11-4
*/
#ifndef LETTUCE_CORE_IGPUMEMORYRESOURCE
#define LETTUCE_CORE_IGPUMEMORYRESOURCE

// project headers
#include "api.hpp"
#include "common.hpp"

namespace Lettuce::Core
{
    /*
    DeviceImpl selects if its host/device and by policy
    so, for raw memory, only size is needed
    */
    struct RawMemory
    {
        VkBuffer buffer;
        uint32_t bufferOffset;
        VkDeviceMemory memory;
        uint32_t memoryOffset;
        uint32_t memorySize;
        HostAddress hostAddress;
        DeviceAddress deviceAddress;
    };

    class IGPUMemoryResource
    {
    public:
        virtual auto AllocateMemory(uint32_t size) -> MemoryView = 0;
        virtual auto AllocateTexture(VkImage) -> bool = 0;
        virtual void ReleaseMemory(MemoryView) = 0;
        virtual void ReleaseTexture(VkImage) = 0;
    };
};
#endif // LETTUCE_CORE_IGPUMEMORYRESOURCE