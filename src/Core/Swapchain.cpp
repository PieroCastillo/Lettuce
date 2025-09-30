// standard headers
#include <limits>
#include <memory>

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
        .hinstance = *(HINSTANCE*)(createInfo.applicationPtr),
        .hwnd = *(HWND*)(createInfo.windowPtr),
    };

    handleResult(vkCreateWin32SurfaceKHR(m_instance, &win32surfaceCI, nullptr, &m_surface));
#endif
#if defined(__linux__)
    VkWaylandSurfaceCreateInfoKHR waylandsurfaceCI = {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .display = (wl_display*)createInfo.applicationPtr,
        .surface = (wl_surface*)createInfo.windowPtr,
    };

    handleResult(vkCreateWaylandSurfaceKHR(m_instance, &waylandsurfaceCI, nullptr, &m_surface));
#endif
}

void Swapchain::setupSwapchain(const SwapchainCreateInfo& createInfo)
{
    // TODO
    VkSwapchainCreateInfoKHR swapchainCI = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,

    };
    if(auto res = vkCreateSwapchainKHR(m_device, &swapchainCI, nullptr, &m_swapchain); res != VK_SUCCESS)
    {

    }
}

void Swapchain::Create(const std::weak_ptr<IDevice>& device, const SwapchainCreateInfo& createInfo)
{
    m_device = (device.lock())->m_device;
    m_gpu = (device.lock())->m_physicalDevice;
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
    if(auto res = vkAcquireNextImageKHR(m_device, m_swapchain, timeout, VK_NULL_HANDLE, VK_NULL_HANDLE, &imgIdx); res != VK_SUCCESS)
    {

    }
}

void Swapchain::Resize(uint32_t newWidth, uint32_t newHeight)
{
}