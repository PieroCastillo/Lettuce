/*
Created by @PieroCastillo on 2025-11-4
*/
#ifndef LETTUCE_CORE_ALLOCATORS_IGPUMEMORYRESOURCE
#define LETTUCE_CORE_ALLOCATORS_IGPUMEMORYRESOURCE

// project headers
#include "../common.hpp"

namespace Lettuce::Core::Allocators
{
    class IGPUMemoryResource
    {
    public:
        virtual BufferAllocation do_balloc(uint32_t size, VkBufferUsageFlags usage) = 0;
        virtual ImageAllocation do_ialloc(VkFormat format, VkExtent2D extent, uint32_t mipLevels, uint32_t arrayLayers, VkImageUsageFlags usage) = 0;
        virtual void do_bdeallocate(const BufferAllocation& allocation) = 0;
        virtual void do_ideallocate(const ImageAllocation& allocation) = 0;

        virtual MemoryAccess GetMemoryAccess() = 0;
    };
};
#endif // LETTUCE_CORE_ALLOCATORS_IGPUMEMORYRESOURCE