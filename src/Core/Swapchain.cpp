// standard headers
#include <limits>

// external headers
#include "Volk/volk.h"

// project headers
#include "Lettuce/Core/Swapchain.hpp"

using namespace Lettuce::Core;

void Swapchain::setupSurface(const SwapchainCreateInfo& createInfo)
{
#if defined(WIN32_) || defined(_WIN32) || defined(WIN32)
    VkWin32SurfaceCreateInfoKHR win32surfaceCI = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = (HINSTANCE)(*createInfo.applicationPtr),
        .hwnd = (HWND)(*createInfo.windowPtr),
    };
    checkResult(vkCreateWin32SurfaceKHR(m_instance, &win32surfaceCI, nullptr, &m_surface));
#endif
#if defined(__linux__)
    VkWaylandSurfaceCreateInfoKHR waylandsurfaceCI = {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .display = (wl_display*)createInfo.applicationPtr,
        .surface = (wl_surface*)createInfo.windowPtr,
    };
    checkResult(vkCreateWaylandSurfaceKHR(m_instance, &waylandsurfaceCI, nullptr, &m_surface));
#endif
}

void Swapchain::setupSwapchain(const SwapchainCreateInfo& createInfo)
{
    VkSwapchainCreateInfoKHR swapchainCI = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,

    };
    checkResult(vkCreateSwapchainKHR(m_device, &swapchainCI, nullptr, &m_swapchain));
}

LettuceResult Swapchain::Create(VkInstance instance, VkPhysicalDevice gpu, VkDevice device, const SwapchainCreateInfo& createInfo)
{
    m_device = device;
    m_gpu = gpu;
    setupSurface(createInfo);
    setupSwapchain(createInfo);

    return LettuceResult::Success;
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
    checkResult(vkAcquireNextImageKHR(m_device, m_swapchain, timeout, VK_NULL_HANDLE, VK_NULL_HANDLE, &imgIdx));
}

void Swapchain::Present()
{
    VkPresentInfoKHR presentI = {

    };
    checkResult(vkQueuePresentKHR());
}

void Swapchain::Resize(uint32_t newWidth, uint32_t newHeight)
{
}