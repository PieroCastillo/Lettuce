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
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Core/common.hpp"

using namespace Lettuce::Core;

void setupVkSurface(SwapchainVK& swapchainVK, VkInstance instance, const SwapchainDesc& createInfo)
{
    VkSurfaceKHR surface;
#if defined(WIN32_) || defined(_WIN32) || defined(WIN32)
    VkWin32SurfaceCreateInfoKHR win32surfaceCI = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = *(HINSTANCE*)(createInfo.applicationPtr),
        .hwnd = *(HWND*)(createInfo.windowPtr),
    };

    handleResult(vkCreateWin32SurfaceKHR(instance, &win32surfaceCI, nullptr, &surface));
#endif
#if defined(__linux__)
    VkWaylandSurfaceCreateInfoKHR waylandsurfaceCI = {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .display = (wl_display*)createInfo.applicationPtr,
        .surface = (wl_surface*)createInfo.windowPtr,
    };

    handleResult(vkCreateWaylandSurfaceKHR(instance, &waylandsurfaceCI, nullptr, &surface));
#endif
    swapchainVK.surface = surface;
}

void setupVkSwapchain(SwapchainVK& swapchainVK, VkDevice device, VkPhysicalDevice gpu, const SwapchainDesc& createInfo)
{
    VkSurfaceKHR surface = swapchainVK.surface;

    // query surface capabilities
    VkSurfaceCapabilitiesKHR sc;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &sc);

    // get surface format and present modes
    uint32_t formatCount;
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, nullptr);
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, formats.data());
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, presentModes.data());

    VkSurfaceFormatKHR surfaceFormat;
    VkExtent2D surfaceExtent;
    VkPresentModeKHR surfacePresentMode;

    surfaceFormat = formats[0];

    surfaceExtent.width = std::clamp(createInfo.width, sc.minImageExtent.width, sc.maxImageExtent.width);
    surfaceExtent.height = std::clamp(createInfo.height, sc.minImageExtent.height, sc.maxImageExtent.height);

    swapchainVK.ltFormat = FromVkFormat(surfaceFormat.format);
    swapchainVK.format = surfaceFormat.format;
    swapchainVK.width = surfaceExtent.width;
    swapchainVK.height = surfaceExtent.height;

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
        .surface = surface,
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

    // TODO: handle errors (as OUT_OF_DATE or SUBOPTIMAL)
    VkSwapchainKHR swapchain;
    auto res = vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &swapchain);
    switch (res)
    {
    case VK_SUCCESS:
    case VK_ERROR_OUT_OF_DATE_KHR:
    case VK_SUBOPTIMAL_KHR:
        break;
    default:
        //handleResult(res);
        break;
    }
    swapchainVK.swapchain = swapchain;
}

void setupImagesAndView(SwapchainVK& swapchainVK, ResourcePool<RenderTarget, RenderTargetVK>& renderTargets, VkDevice device, VkPhysicalDevice gpu, const SwapchainDesc& createInfo)
{
    auto swapchain = swapchainVK.swapchain;
    // get swapchain images
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    swapchainVK.swapchainImages.resize(imageCount, VK_NULL_HANDLE);
    swapchainVK.imageCount = imageCount;
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainVK.swapchainImages.data());

    // create views
    VkImageViewCreateInfo viewCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = swapchainVK.format,
        .components = { VK_COMPONENT_SWIZZLE_IDENTITY , VK_COMPONENT_SWIZZLE_IDENTITY , VK_COMPONENT_SWIZZLE_IDENTITY ,VK_COMPONENT_SWIZZLE_IDENTITY },
        .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },// 1 mipmap, 1 layer
    };

    // create Swapchain Image Views
    swapchainVK.swapchainViews.resize(swapchainVK.imageCount);
    swapchainVK.renderTargets.resize(swapchainVK.imageCount);

    VkImageView view;
    for (uint32_t i = 0; i < swapchainVK.imageCount; ++i)
    {
        viewCI.image = swapchainVK.swapchainImages[i];
        handleResult(vkCreateImageView(device, &viewCI, nullptr, &view));

        auto renderView = renderTargets.allocate({
            true, swapchainVK.width, swapchainVK.height, swapchainVK.format, swapchainVK.swapchainImages[i], view,
            VK_NULL_HANDLE, 0, 0
            });

        swapchainVK.swapchainViews[i] = view;
        swapchainVK.renderTargets[i] = renderView;
    }
}

Swapchain Device::CreateSwapchain(const SwapchainDesc& desc)
{
    auto device = impl->m_device;
    auto gpu = impl->m_physicalDevice;
    auto instance = impl->m_instance;

    SwapchainVK swapchainVK = {};
    swapchainVK.currentImageIndex = 0;
    setupVkSurface(swapchainVK, instance, desc);
    setupVkSwapchain(swapchainVK, device, gpu, desc);
    setupImagesAndView(swapchainVK, impl->renderTargets, device, gpu, desc);
    VkFenceCreateInfo fenceCI = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };
    handleResult(vkCreateFence(device, &fenceCI, nullptr, &swapchainVK.waitForAcquireFence));
    VkSemaphoreCreateInfo semCI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    swapchainVK.presentSemaphores.resize(swapchainVK.imageCount);
    for (int i = 0; i < swapchainVK.presentSemaphores.size(); ++i)
    {
        handleResult(vkCreateSemaphore(device, &semCI, nullptr, &swapchainVK.presentSemaphores[i]));
    }

    return impl->swapchains.allocate(std::move(swapchainVK));
}

void Device::Destroy(Swapchain swapchain)
{
    auto info = impl->swapchains.get(swapchain);
    auto device = impl->m_device;
    for (const auto& sem : info.presentSemaphores)
    {
        vkDestroySemaphore(device, sem, nullptr);
    }
    vkDestroyFence(device, info.waitForAcquireFence, nullptr);
    for (int i = 0; i < info.swapchainViews.size(); ++i)
    {
        vkDestroyImageView(device, info.swapchainViews[i], nullptr);
        impl->renderTargets.free(info.renderTargets[i]);
    }
    info.swapchainViews.clear();
    info.swapchainImages.clear();
    vkDestroySwapchainKHR(device, info.swapchain, nullptr);
    vkDestroySurfaceKHR(impl->m_instance, info.surface, nullptr);
}

void Device::NextFrame(Swapchain swapchain)
{
    auto& info = impl->swapchains.get(swapchain);
    auto device = impl->m_device;
    vkResetFences(device, 1, &info.waitForAcquireFence);
    constexpr auto timeout = (std::numeric_limits<uint32_t>::max)();
    vkAcquireNextImageKHR(device, info.swapchain, timeout, VK_NULL_HANDLE, info.waitForAcquireFence, &info.currentImageIndex);
    // if (auto res = vkAcquireNextImageKHR(device, info.swapchain, timeout, VK_NULL_HANDLE, info.waitForAcquireFence, &info.currentImageIndex); res != VK_SUCCESS)
    // {

    // }
    handleResult(vkWaitForFences(device, 1, &info.waitForAcquireFence, VK_TRUE, timeout));
}

void Device::DisplayFrame(Swapchain swapchain)
{
    auto& info = impl->swapchains.get(swapchain);
    // TODO: Manage Global Synchronization
    VkPresentInfoKHR presentI = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &info.presentSemaphores[(int)(info.currentImageIndex)],
        .swapchainCount = 1,
        .pSwapchains = &info.swapchain,
        .pImageIndices = &info.currentImageIndex,
    };
    handleResult(vkQueueWaitIdle(impl->m_graphicsQueue));
    handleResult(vkQueuePresentKHR(impl->m_graphicsQueue, &presentI));
    // wait for present complete
    info.currentImageIndex = (info.currentImageIndex + 1) % info.imageCount;
}

Format Device::GetRenderTargetFormat(Swapchain swapchain)
{
    auto& swp = impl->swapchains.get(swapchain);
    return swp.ltFormat;
}

void Device::ResizeSwapchain(Swapchain swapchain, uint32_t w, uint32_t h)
{
    // TODO: impl
}

RenderTarget Device::GetCurrentRenderTarget(Swapchain swapchain) const
{
    auto& swc = impl->swapchains.get(swapchain);
    return swc.renderTargets[(int)swc.currentImageIndex];
}