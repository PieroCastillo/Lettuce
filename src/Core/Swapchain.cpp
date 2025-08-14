// standard headers
#include <limits>

// external headers
#include "Volk/volk.h"

// project headers
#include "Lettuce/Core/Swapchain.hpp"

using namespace Lettuce::Core;

void Swapchain::setupSurface(const SwapchainCreateInfo &createInfo)
{

}

void Swapchain::setupSwapchain(const SwapchainCreateInfo &createInfo)
{
    VkSwapchainCreateInfoKHR swapchainCI = {

    };
    vkCreateSwapchainKHR(m_device, &swapchainCI, nullptr, &m_swapchain);
}

Swapchain::Swapchain(VkDevice device, VkPhysicalDevice gpu, const SwapchainCreateInfo &createInfo)
{
    setupSurface(createInfo);
    setupSwapchain(createInfo);
}

void Swapchain::Release()
{
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
}

void Swapchain::AcquireNextImage()
{
    constexpr auto timeout = std::numeric_limits<uint32_t>::max();
    uint32_t imgIdx = 0;
    vkAcquireNextImageKHR(m_device, m_swapchain, timeout, VK_NULL_HANDLE, VK_NULL_HANDLE, &imgIdx);
}

void Swapchain::Present()
{
}

void Swapchain::Resize(uint32_t newWidth, uint32_t newHeight)
{
}

void Swapchain::Wait()
{
}