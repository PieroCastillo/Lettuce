/*
Created by @PieroCastillo on 2025-10-22
*/
#ifndef LETTUCE_CORE_FRAME_CONTEXT_HPP
#define LETTUCE_CORE_FRAME_CONTEXT_HPP

// standard headers
#include <vector>

// project headers
#include "common.hpp"

namespace Lettuce::Core
{
    struct FrameContextCreateInfo
    {
        int semaphoreCount;
    };

    /*
    Stores information for Commands Execution and Synchronization
    */
    class FrameContext
    {
    public:
        VkDevice m_device;
        std::vector<VkSemaphore> m_semaphores;
        VkSemaphore m_presentSemaphore;

        void Create(const IDevice& device, const FrameContextCreateInfo& createInfo);
        void Release();
    };
};

#endif // LETTUCE_CORE_FRAME_CONTEXT_HPP