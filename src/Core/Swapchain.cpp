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

    // get surface format and present modes
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
    m_format = surfaceFormat.format;

    surfaceExtent.width = std::clamp(createInfo.width, sc.minImageExtent.width, sc.maxImageExtent.width);
    surfaceExtent.height = std::clamp(createInfo.height, sc.minImageExtent.height, sc.maxImageExtent.height);

    // set internal extent
    m_width = surfaceExtent.width;
    m_height = surfaceExtent.height;

    // get most appropiate present mode
    for (int i = 0; i < presentModeCount; i++)
    {
        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            surfacePresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
        else if (presentModes[i] == VK_PRESENT_MODE_FIFO_KHR)
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

void Swapchain::setupImagesAndView(const SwapchainCreateInfo& createInfo)
{
    // get swapchain images
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_imageCount, nullptr);
    m_swapchainImages.resize(m_imageCount, VK_NULL_HANDLE);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_imageCount, m_swapchainImages.data());

    // create views
    VkImageViewCreateInfo viewCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = m_format,
        .components = { VK_COMPONENT_SWIZZLE_IDENTITY , VK_COMPONENT_SWIZZLE_IDENTITY , VK_COMPONENT_SWIZZLE_IDENTITY ,VK_COMPONENT_SWIZZLE_IDENTITY },
        .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },// 1 mipmap, 1 layer
    };

    // create Swapchain Image Views
    m_swapchainViews.reserve(m_imageCount);
    for (uint32_t i = 0; i < m_imageCount; ++i)
    {
        viewCI.image = m_swapchainImages[i];
        VkImageView view;
        handleResult(vkCreateImageView(m_device, &viewCI, nullptr, &view));
        m_swapchainViews.push_back(view);
    }
}

void Swapchain::Create(const IDevice& device, const SwapchainCreateInfo& createInfo)
{
    m_device = device.m_device;
    m_gpu = device.m_physicalDevice;
    m_instance = device.m_instance;
    // graphics queue always support present
    m_presentQueue = device.m_graphicsQueue;
    m_currentImageIndex = 0;
    setupSurface(createInfo);
    setupSwapchain(createInfo);
    setupImagesAndView(createInfo);
    VkFenceCreateInfo fenceCI = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };
    handleResult(vkCreateFence(m_device, &fenceCI, nullptr, &m_waitForAcquireFence));
}

void Swapchain::Release()
{
    vkDestroyFence(m_device, m_waitForAcquireFence, nullptr);
    for (const auto& view : m_swapchainViews)
    {
        vkDestroyImageView(m_device, view, nullptr);
    }
    m_swapchainViews.clear();
    m_swapchainImages.clear();
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
}

void Swapchain::NextFrame()
{
    vkResetFences(m_device, 1, &m_waitForAcquireFence);
    constexpr auto timeout = (std::numeric_limits<uint32_t>::max)();
    if (auto res = vkAcquireNextImageKHR(m_device, m_swapchain, timeout, VK_NULL_HANDLE, m_waitForAcquireFence, &m_currentImageIndex); res != VK_SUCCESS)
    {

    }
    handleResult(vkWaitForFences(m_device, 1, &m_waitForAcquireFence, VK_TRUE, timeout));
}

void Swapchain::DisplayFrame()
{
    // TODO: Manage Global Synchronization
    VkPresentInfoKHR presentI = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .swapchainCount = 1,
        .pSwapchains = &m_swapchain,
        .pImageIndices = &m_currentImageIndex,
    };
    handleResult(vkQueuePresentKHR(m_presentQueue, &presentI));
    // wait for present complete
    handleResult(vkQueueWaitIdle(m_presentQueue));
    // m_currentImageIndex = (m_currentImageIndex + 1) % m_imageCount;
}

void Swapchain::Resize(uint32_t newWidth, uint32_t newHeight)
{
    // TODO: impl
}