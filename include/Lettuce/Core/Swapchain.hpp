/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_SWAPCHAIN_HPP
#define LETTUCE_CORE_SWAPCHAIN_HPP

namespace Lettuce::Core
{
    struct SwapchainCreateInfo
    {
        uint32_t width;
        uint32_t height;
        bool clipped;
    };

    class Swapchain
    {
    private:
        uint32_t width;
        uint32_t height;

        inline void setupSurface(const SwapchainCreateInfo &createInfo);
        inline void setupSwapchain(const SwapchainCreateInfo &createInfo);

    public:
        VkDevice m_device;
        VkPhysicalDevice m_gpu;
        VkSwapchainKHR m_swapchain;
        VkSurfaceKHR m_surface;

        Swapchain(VkDevice device, VkPhysicalDevice gpu, const SwapchainCreateInfo &createInfo);
        void Release();

        void AcquireNextImage();
        void Present();
        void Resize(uint32_t newWidth, uint32_t newHeight);
        void Wait();
    };
}
#endif // LETTUCE_CORE_SWAPCHAIN_HPP