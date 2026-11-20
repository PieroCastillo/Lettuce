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
#include <vulkan/vulkan.h>
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

    auto vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
    handleResult(vkCreateWin32SurfaceKHR(instance, &win32surfaceCI, nullptr, &surface));
#elifdef __linux__
    VkWaylandSurfaceCreateInfoKHR waylandsurfaceCI = {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .display = (wl_display*)createInfo.applicationPtr,
        .surface = (wl_surface*)createInfo.windowPtr,
    };

    auto vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWaylandSurfaceKHR");
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

    VkSurfaceCapabilitiesKHR sc{};
    handleResult(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &sc));

    // Surface formats
    uint32_t formatCount = 0;
    handleResult(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, nullptr));

    if (formatCount == 0)
        throw std::runtime_error("Surface has no supported formats.");

    std::vector<VkSurfaceFormatKHR> formats(formatCount);

    handleResult(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, formats.data()));

    // Present modes
    uint32_t presentModeCount = 0;
    handleResult(vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, nullptr));

    if (presentModeCount == 0)
        throw std::runtime_error("Surface has no supported present modes.");

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);

    handleResult(vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, presentModes.data()));

    // Select surface format
    VkSurfaceFormatKHR surfaceFormat = formats[0];

    if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
        surfaceFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

    // Select present mode: MAILBOX > FIFO
    VkPresentModeKHR surfacePresentMode =
        VK_PRESENT_MODE_FIFO_KHR;

    if (std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_MAILBOX_KHR) != presentModes.end())
        surfacePresentMode = VK_PRESENT_MODE_MAILBOX_KHR;

    // Select extent
    VkExtent2D surfaceExtent{};

    if (sc.currentExtent.width != UINT32_MAX)
    {
        surfaceExtent = sc.currentExtent;
    }
    else
    {
        surfaceExtent.width = std::clamp(desiredWidth, sc.minImageExtent.width, sc.maxImageExtent.width);
        surfaceExtent.height = std::clamp(desiredHeight, sc.minImageExtent.height, sc.maxImageExtent.height);
    }

    if (surfaceExtent.width == 0 || surfaceExtent.height == 0)
        throw std::runtime_error("Invalid swapchain extent.");

    // Select image count
    uint32_t imageCount = sc.minImageCount + 1;

    if (sc.maxImageCount != 0)
        imageCount = std::min(imageCount, sc.maxImageCount);

    // Select image usage
    VkImageUsageFlags requiredUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if ((sc.supportedUsageFlags & requiredUsage) != requiredUsage)
        throw std::runtime_error("Surface does not support COLOR_ATTACHMENT usage.");

    // Preserve optional usages when supported.
    VkImageUsageFlags optionalUsage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkImageUsageFlags imageUsage = requiredUsage | (optionalUsage & sc.supportedUsageFlags);

    // Select composite alpha
    constexpr VkCompositeAlphaFlagBitsKHR alphaModes[] = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };

    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    bool foundAlpha = false;

    for (auto mode : alphaModes)
    {
        if (sc.supportedCompositeAlpha & mode)
        {
            compositeAlpha = mode;
            foundAlpha = true;
            break;
        }
    }

    if (!foundAlpha)
        throw std::runtime_error("No supported composite alpha mode.");

    // Store selected configuration
    swapchainVK.ltFormat = FromVkFormat(surfaceFormat.format);
    swapchainVK.format = surfaceFormat.format;
    swapchainVK.width = surfaceExtent.width;
    swapchainVK.height = surfaceExtent.height;
    swapchainVK.clipped = createInfo.clipped;

    VkSwapchainCreateInfoKHR swapchainCI = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = surfaceExtent,
        .imageArrayLayers = 1,
        .imageUsage = imageUsage,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = sc.currentTransform,
        .compositeAlpha = compositeAlpha,
        .presentMode = surfacePresentMode,
        .clipped = createInfo.clipped,
        .oldSwapchain = oldSwapchain,
    };

    VkSwapchainKHR newSwapchain = VK_NULL_HANDLE;
    handleResult(vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &newSwapchain));

    // Destroy old resources only after successful creation.
    if (oldSwapchain != VK_NULL_HANDLE)
    {
        for (size_t i = 0; i < swapchainVK.swapchainViews.size(); ++i)
        {
            vkDestroyImageView(device, swapchainVK.swapchainViews[i], nullptr);
            impl->textures.release(swapchainVK.renderTargets[i]);
        }

        swapchainVK.renderTargets.clear();
        swapchainVK.swapchainViews.clear();
        swapchainVK.swapchainImages.clear();

        vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
    }

    swapchainVK.swapchain = newSwapchain;
}

void setupImagesAndView(DeviceImpl* impl, SwapchainVK& swapchainVK, const SwapchainDesc& createInfo)
{
    auto& textures = impl->textures;
    auto device = impl->m_device;
    auto gpu = impl->m_physicalDevice;
    auto swapchain = swapchainVK.swapchain;

    uint32_t imageCount = 0;
    handleResult(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr));

    if (imageCount == 0)
        throw std::runtime_error("Swapchain has no images.");

    swapchainVK.swapchainImages.resize(imageCount);
    handleResult(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainVK.swapchainImages.data()));
    swapchainVK.imageCount = imageCount;

    VkImageViewCreateInfo viewCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = swapchainVK.format,
        .components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY },
        .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
    };

    swapchainVK.swapchainViews.resize(imageCount);
    swapchainVK.renderTargets.resize(imageCount);

    constexpr uint64_t timeout = UINT64_MAX;
    for (uint32_t i = 0; i < imageCount; ++i)
    {
        viewCI.image = swapchainVK.swapchainImages[i];

        VkImageView view = VK_NULL_HANDLE;
        handleResult(vkCreateImageView(device, &viewCI, nullptr, &view));

        auto texView = textures.allocate({ swapchainVK.width, swapchainVK.height, 1, 1, swapchainVK.format, swapchainVK.swapchainImages[i], view,
                                          VK_NULL_HANDLE, 0, 0, VK_NULL_HANDLE, nullptr, true, ToVkClearValue(createInfo.targetColor) });

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
    VkFenceCreateInfo fenceCI = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };
    handleResult(vkCreateFence(device, &fenceCI, nullptr, &swapchainVK.waitForAcquireFence));
    setupImagesAndView(impl, swapchainVK, desc);

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
    constexpr uint64_t timeout = UINT64_MAX;

    // Reset fence before acquiring
    handleResult(vkResetFences(device, 1, &info.waitForAcquireFence));

    auto res = vkAcquireNextImageKHR(device, info.swapchain, timeout, VK_NULL_HANDLE, info.waitForAcquireFence, &info.currentImageIndex);
    bool recreate = false;

    if (res == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreate = true;
    }
    else if (res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR)
    {
        handleResult(vkWaitForFences(device, 1, &info.waitForAcquireFence, VK_TRUE, timeout));
        VkSurfaceCapabilitiesKHR sc{};
        handleResult(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(impl->m_physicalDevice, info.surface, &sc));

        // Dynamic extent: recreate when requested size changes
        if (sc.currentExtent.width == UINT32_MAX && (info.width != desiredWidth || info.height != desiredHeight))
            recreate = true;
    }
    else
    {
        handleResult(res);
    }

    if (recreate)
    {
        // recreateSwapchain
        handleResult(vkQueueWaitIdle(impl->m_graphicsQueue));

        SwapchainDesc desc{ .clipped = info.clipped };
        setupVkSwapchain(info, impl, impl->m_physicalDevice, desc, desiredWidth, desiredHeight);
        setupImagesAndView(impl, info, desc);

        // If acquire previously succeeded, its fence is signaled
        // If it returned OUT_OF_DATE, the fence is still unsignaled
        if (res != VK_ERROR_OUT_OF_DATE_KHR)
            handleResult(vkResetFences(device, 1, &info.waitForAcquireFence));

        res = vkAcquireNextImageKHR(device, info.swapchain, timeout, VK_NULL_HANDLE, info.waitForAcquireFence, &info.currentImageIndex);

        if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
            handleResult(res);

        handleResult(vkWaitForFences(device, 1, &info.waitForAcquireFence, VK_TRUE, timeout));
    }

    return { info.width,info.height };
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
        //   info.currentImageIndex = (info.currentImageIndex + 1) % info.imageCount;
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