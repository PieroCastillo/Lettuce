// standard headers
#include <limits>
#include <memory>
#include <vector>
#include <algorithm>

// external headers
#if defined(WIN32_) || defined(_WIN32) || defined(WIN32)
    #define VK_USE_PLATFORM_WIN32_KHR
    #include <windows.h>
#endif

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
    // query surface capabilities
    VkSurfaceCapabilitiesKHR sc;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_gpu, m_surface, &sc);

    uint32_t formatCount;
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpu, m_surface, &formatCount, nullptr);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_gpu, m_surface, &presentModeCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpu, m_surface, &formatCount, formats.data());
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_gpu, m_surface, &presentModeCount, presentModes.data());

    VkSurfaceFormatKHR surfaceFormat;
    VkExtent2D surfaceExtent;
    VkPresentModeKHR surfacePresentMode;

    surfaceFormat = formats[0];

    surfaceExtent.width = std::clamp(createInfo.width, sc.minImageExtent.width, sc.maxImageExtent.width);
    surfaceExtent.height = std::clamp(createInfo.height, sc.minImageExtent.height, sc.maxImageExtent.height);

    // set internal extent
    m_width = surfaceExtent.width;
    m_height = surfaceExtent.height;

    for(int i = 0; i < presentModeCount; i++)
    {
        if(presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            surfacePresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
        else if(presentModes[i] == VK_PRESENT_MODE_FIFO_KHR)
        {
            surfacePresentMode = VK_PRESENT_MODE_FIFO_KHR;
        }
        else
        {
            surfacePresentMode = presentModes[0];
        }
    }

    // init values for swapchain creation
    VkSwapchainCreateInfoKHR swapchainCI = 
    {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = m_surface,
        .minImageCount = sc.minImageCount + 1,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = surfaceExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = sc.currentTransform ,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR ,
        .presentMode = surfacePresentMode,
        .clipped = VK_FALSE,
        .oldSwapchain = VK_NULL_HANDLE,
    };
    if (auto res = vkCreateSwapchainKHR(m_device, &swapchainCI, nullptr, &m_swapchain); res != VK_SUCCESS)
    {
        // TODO: handle errors (as OUT_OF_DATE or SUBOPTIMAL)
    }
}

void Swapchain::Create(const IDevice& device, const SwapchainCreateInfo& createInfo)
{
    m_device = device.m_device;
    m_gpu = device.m_physicalDevice;
    m_instance = device.m_instance;
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
    constexpr auto timeout = (std::numeric_limits<uint32_t>::max)();
    uint32_t imgIdx = 0;
    if (auto res = vkAcquireNextImageKHR(m_device, m_swapchain, timeout, VK_NULL_HANDLE, VK_NULL_HANDLE, &imgIdx); res != VK_SUCCESS)
    {

    }
}

void Swapchain::Resize(uint32_t newWidth, uint32_t newHeight)
{
    // TODO: impl
}