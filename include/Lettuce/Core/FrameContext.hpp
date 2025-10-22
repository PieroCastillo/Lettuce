/*
Created by @PieroCastillo on 2025-10-22
*/
#ifndef FRAME_CONTEXT_HPP
#define FRAME_CONTEXT_HPP

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

#endif // FRAME_CONTEXT_HPP