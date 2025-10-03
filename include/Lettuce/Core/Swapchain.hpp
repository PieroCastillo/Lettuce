/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_SWAPCHAIN_HPP
#define LETTUCE_CORE_SWAPCHAIN_HPP

// project headers
#include "common.hpp"

namespace Lettuce::Core
{
    struct SwapchainCreateInfo
    {
        uint32_t width;
        uint32_t height;
        bool clipped;
        void* windowPtr;
        void* applicationPtr;
    };

    class Swapchain
    {
    private:
        uint32_t width;
        uint32_t height;

        inline void setupSurface(const SwapchainCreateInfo &createInfo);
        inline void setupSwapchain(const SwapchainCreateInfo &createInfo);

    public:
        VkInstance m_instance;
        VkPhysicalDevice m_gpu;
        VkDevice m_device;
        VkSwapchainKHR m_swapchain;
        VkSurfaceKHR m_surface;

        void Create(const std::weak_ptr<IDevice>& device, const SwapchainCreateInfo &createInfo);
        void Release();

        void AcquireNextImage();
        void Resize(uint32_t newWidth, uint32_t newHeight);
    };
}
#endif // LETTUCE_CORE_SWAPCHAIN_HPP