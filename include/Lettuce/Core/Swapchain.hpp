/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_SWAPCHAIN_HPP
#define LETTUCE_CORE_SWAPCHAIN_HPP

// standard headers
#include <vector>
#include <memory>

// project headers
#include "common.hpp"
#include "RenderTarget.hpp"

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
        uint32_t m_width;
        uint32_t m_height;
        std::vector<VkImage> m_swapchainImages;
        std::vector<VkImageView> m_swapchainViews;
        std::vector<std::shared_ptr<RenderTarget>> m_renderViews;

        void setupSurface(const SwapchainCreateInfo &createInfo);
        void setupSwapchain(const SwapchainCreateInfo &createInfo);
        void setupImagesAndView(const SwapchainCreateInfo &createInfo);

    public:
        VkInstance m_instance;
        VkPhysicalDevice m_gpu;
        VkDevice m_device;
        VkQueue m_presentQueue;
        VkFormat m_format;
        VkSwapchainKHR m_swapchain;
        VkSurfaceKHR m_surface;
        VkFence m_waitForAcquireFence;
        uint32_t m_imageCount;
        uint32_t m_currentImageIndex;

        void Create(const IDevice& device, const SwapchainCreateInfo &createInfo);
        void Release();

        void NextFrame();
        void DisplayFrame();
        void Resize(uint32_t newWidth, uint32_t newHeight);
        const RenderTarget& GetCurrentRenderView();
        std::vector<std::weak_ptr<RenderTarget>> GetRenderViews();

        inline uint32_t GetHeight() { return m_height; }
        inline uint32_t GetWidth() { return m_width; }
    };
}
#endif // LETTUCE_CORE_SWAPCHAIN_HPP