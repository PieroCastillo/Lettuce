// standard headers
#include <limits>
#include <memory>
#include <vector>
#include <algorithm>

// external headers
#if defined(WIN32_) || defined(_WIN32) || defined(WIN32)
#include <windows.h>
#elifdef __linux__
#include <wayland-client.h>
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
        .hinstance = (HINSTANCE)(createInfo.applicationPtr),
        .hwnd = (HWND)(createInfo.windowPtr),
    };

    handleResult(vkCreateWin32SurfaceKHR(instance, &win32surfaceCI, nullptr, &surface));
#elifdef __linux__
    VkWaylandSurfaceCreateInfoKHR waylandsurfaceCI = {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .display = (wl_display*)createInfo.applicationPtr,
        .surface = (wl_surface*)createInfo.windowPtr,
    };

    handleResult(vkCreateWaylandSurfaceKHR(instance, &waylandsurfaceCI, nullptr, &surface));
#endif
    swapchainVK.surface = surface;
}

void setupVkSwapchain(SwapchainVK& swapchainVK, DeviceImpl* impl, VkPhysicalDevice gpu, const SwapchainDesc& createInfo,
    uint32_t desiredWidth = 0, uint32_t desiredHeight = 0)
{
    VkDevice device = impl->m_device;
    VkSurfaceKHR surface = swapchainVK.surface;
    VkSwapchainKHR oldSwapchain = swapchainVK.swapchain;

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

    /*
    windows: currentExtent is contains the actual framebuffer size
    linux/wayland: currentExtend is "dynamic" (gives you a uint32Max) and swapchain can decide framebuffer size by "itself"
    btw due to this i need to impl AGAIN width/height to Device::NextFrame(...)
    */
    if (sc.currentExtent.width == UINT32_MAX & sc.currentExtent.height == UINT32_MAX) {
        surfaceExtent.width = std::clamp(desiredWidth, sc.minImageExtent.width, sc.maxImageExtent.width);
        surfaceExtent.height = std::clamp(desiredHeight, sc.minImageExtent.height, sc.maxImageExtent.height);
    }
    else {
        surfaceExtent.width = std::clamp(sc.currentExtent.width, sc.minImageExtent.width, sc.maxImageExtent.width);
        surfaceExtent.height = std::clamp(sc.currentExtent.height, sc.minImageExtent.height, sc.maxImageExtent.height);
    }

    swapchainVK.ltFormat = FromVkFormat(surfaceFormat.format);
    swapchainVK.format = surfaceFormat.format;
    swapchainVK.width = surfaceExtent.width;
    swapchainVK.height = surfaceExtent.height;
    swapchainVK.clipped = createInfo.clipped;

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
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = sc.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = surfacePresentMode,
        .clipped = createInfo.clipped,
        .oldSwapchain = oldSwapchain,
    };

    // TODO: handle errors (as OUT_OF_DATE or SUBOPTIMAL)
    VkSwapchainKHR swapchain;
    auto res = vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &swapchain);
    switch (res)
    {
    case VK_SUCCESS:
    case VK_SUBOPTIMAL_KHR:
        break;
    default:
        //handleResult(res);
        break;
    }
    swapchainVK.swapchain = swapchain;

    if (oldSwapchain != VK_NULL_HANDLE) {
        for (int i = 0; i < swapchainVK.swapchainViews.size(); ++i)
        {
            vkDestroyImageView(device, swapchainVK.swapchainViews[i], nullptr);
            impl->textures.release(swapchainVK.renderTargets[i]);
        }
        swapchainVK.renderTargets.clear();
        swapchainVK.swapchainViews.clear();
        swapchainVK.swapchainImages.clear();
        vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
    }
}

void setupImagesAndView(SwapchainVK& swapchainVK, ResourcePool<TextureView, TextureVK>& textures, VkDevice device, VkPhysicalDevice gpu, const SwapchainDesc& createInfo)
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

        auto img = swapchainVK.swapchainImages[i];
        auto texView = textures.allocate({ swapchainVK.width, swapchainVK.height, 1, 1,
                                            swapchainVK.format, img, view,VK_NULL_HANDLE,
                                            0, 0, VK_NULL_HANDLE, nullptr,
                                            true });

        swapchainVK.swapchainViews[i] = view;
        swapchainVK.renderTargets[i] = texView;
    }
}

auto Device::CreateSwapchain(const SwapchainDesc& desc) -> Swapchain
{
    auto device = impl->m_device;
    auto gpu = impl->m_physicalDevice;
    auto instance = impl->m_instance;

    SwapchainVK swapchainVK = {};
    swapchainVK.currentImageIndex = 0;
    setupVkSurface(swapchainVK, instance, desc);
    setupVkSwapchain(swapchainVK, impl, gpu, desc, 1000, 1000);
    setupImagesAndView(swapchainVK, impl->textures, device, gpu, desc);
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
    auto& info = impl->swapchains.get(swapchain);
    auto& device = impl->m_device;
    for (const auto& sem : info.presentSemaphores)
    {
        vkDestroySemaphore(device, sem, nullptr);
    }
    vkDestroyFence(device, info.waitForAcquireFence, nullptr);
    for (int i = 0; i < info.swapchainViews.size(); ++i)
    {
        vkDestroyImageView(device, info.swapchainViews[i], nullptr);
        impl->textures.release(info.renderTargets[i]);
    }
    info.swapchainViews.clear();
    info.swapchainImages.clear();
    vkDestroySwapchainKHR(device, info.swapchain, nullptr);
    vkDestroySurfaceKHR(impl->m_instance, info.surface, nullptr);
}

auto Device::NextFrame(Swapchain swapchain, uint32_t desiredWidth, uint32_t desiredHeight) -> Size
{
    auto& info = impl->swapchains.get(swapchain);
    auto device = impl->m_device;
    vkResetFences(device, 1, &info.waitForAcquireFence);
    constexpr auto timeout = (std::numeric_limits<uint32_t>::max)();
    auto res = vkAcquireNextImageKHR(device, info.swapchain, timeout, VK_NULL_HANDLE, info.waitForAcquireFence, &info.currentImageIndex);

    VkSurfaceCapabilitiesKHR sc;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(impl->m_physicalDevice, info.surface, &sc);

    if (res == VK_ERROR_OUT_OF_DATE_KHR ||
        (sc.currentExtent.width == UINT32_MAX && (info.width != desiredWidth || info.height != desiredHeight)))
    {
        handleResult(vkQueueWaitIdle(impl->m_graphicsQueue));
        SwapchainDesc desc = { .clipped = info.clipped };
        if (res != VK_ERROR_OUT_OF_DATE_KHR)
            vkWaitForFences(device, 1, &info.waitForAcquireFence, VK_TRUE, timeout);
        setupVkSwapchain(info, impl, impl->m_physicalDevice, desc, desiredWidth, desiredHeight);
        setupImagesAndView(info, impl->textures, device, impl->m_physicalDevice, desc);
        if (res != VK_ERROR_OUT_OF_DATE_KHR)
            vkResetFences(device, 1, &info.waitForAcquireFence);
        handleResult(vkAcquireNextImageKHR(device, info.swapchain, timeout, VK_NULL_HANDLE, info.waitForAcquireFence, &info.currentImageIndex));
    }

    handleResult(vkWaitForFences(device, 1, &info.waitForAcquireFence, VK_TRUE, timeout));

    return { info.width, info.height };
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
    // TODO: further, we need to replace this usign a better sync system
    // handleResult(vkQueueWaitIdle(impl->m_graphicsQueue));
    auto res = vkQueuePresentKHR(impl->m_graphicsQueue, &presentI);
    switch (res)
    {
    case VK_SUCCESS:
    case VK_SUBOPTIMAL_KHR:
    case VK_ERROR_OUT_OF_DATE_KHR:
        info.currentImageIndex = (info.currentImageIndex + 1) % info.imageCount;
        break;

    default:
        handleResult(res);
        break;
    }
}

auto Device::GetRenderTargetFormat(Swapchain swapchain) -> Format
{
    auto& swp = impl->swapchains.get(swapchain);
    return swp.ltFormat;
}

auto Device::GetCurrentRenderTarget(Swapchain swapchain) const -> TextureView
{
    auto& swc = impl->swapchains.get(swapchain);
    return swc.renderTargets[(int)swc.currentImageIndex];
}

auto Device::GetFrameCount(Swapchain swapchain) -> uint32_t
{
    auto& swc = impl->swapchains.get(swapchain);
    return swc.swapchainImages.size();
}