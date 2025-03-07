//
// Created by piero on 14/02/2024.
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <tuple>
#include <algorithm>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/Swapchain.hpp"

using namespace Lettuce::Core;

void Swapchain::loadImages()
{
    vkGetSwapchainImagesKHR(_device->_device, _swapchain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_device->_device, _swapchain, &imageCount, swapChainImages.data());
}

void Swapchain::createImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo imageViewCI = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapChainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = imageFormat,
            .components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        };
        checkResult(vkCreateImageView(_device->_device, &imageViewCI, nullptr, &swapChainImageViews[i]));
    }
}

Swapchain::Swapchain(const std::shared_ptr<Device> &device, uint32_t initialWidth, uint32_t initialHeight) : _device(device)
{
    auto capabilities = _device->_gpu.capabilities;
    imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
    {
        imageCount = capabilities.maxImageCount;
    }
    imageFormat = _device->_gpu.surfaceFormat.format;
    width = initialWidth;
    height = initialHeight;
    extent = {initialWidth, initialHeight};
    VkSwapchainCreateInfoKHR swapchainCI = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = _device->_instance->_surface,
        .minImageCount = imageCount,
        .imageFormat = imageFormat,
        .imageColorSpace = _device->_gpu.surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        // .queueFamilyIndexCount;
        // .pQueueFamilyIndices;
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = _device->_gpu.presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    std::vector<uint32_t> queueFamilyIndices;
    queueFamilyIndices.push_back(_device->_gpu.graphicsFamily.value());
    queueFamilyIndices.push_back(_device->_gpu.presentFamily.value());

    if (_device->_gpu.graphicsFamily.value() != _device->_gpu.presentFamily.value())
    {
        swapchainCI.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCI.queueFamilyIndexCount = (uint32_t)queueFamilyIndices.size();
        swapchainCI.pQueueFamilyIndices = queueFamilyIndices.data();
    }
    else
    {
        swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    checkResult(vkCreateSwapchainKHR(_device->_device, &swapchainCI, nullptr, &_swapchain));

    VkFenceCreateInfo fenceCI = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };
    checkResult(vkCreateFence(_device->_device, &fenceCI, nullptr, &_fence));

    loadImages();
    createImageViews();
}

void Swapchain::AcquireNextImage()
{
    checkResult(vkResetFences(_device->_device, 1, &_fence));
    checkResult(vkAcquireNextImageKHR(_device->_device, _swapchain, (std::numeric_limits<uint64_t>::max)(), VK_NULL_HANDLE, _fence, &index));
    checkResult(vkWaitForFences(_device->_device, 1, &_fence, VK_TRUE, (std::numeric_limits<uint64_t>::max)()));
}

void Swapchain::Present()
{
    VkPresentInfoKHR presentI = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .swapchainCount = 1,
        .pSwapchains = &_swapchain,
        .pImageIndices = &index,
    };
    auto result = vkQueuePresentKHR(_device->_presentQueue, &presentI);

    switch (result)
    {
    case VK_SUCCESS:
        break;
    case VK_ERROR_OUT_OF_DATE_KHR:
    case VK_SUBOPTIMAL_KHR:
    {
        auto [w, h] = _func();
        Resize(w, h);
        _postFunc();
        break;
    }
    default:
        checkResult(result);
        break;
    }

    checkResult(vkQueueWaitIdle(_device->_presentQueue));
}

void Swapchain::Wait()
{
    // VkPresentIdKHR s = {
    // };
    // vkWaitForPresentKHR(_device->_device, _swapchain, )
}

void Swapchain::SetResizeFunc(std::function<std::tuple<uint32_t, uint32_t>(void)> call, std::function<void(void)> postFunc)
{
    _func = call;
    _postFunc = postFunc;
}

void Swapchain::Resize(uint32_t newWidth, uint32_t newHeight)
{
    for (auto imageView : swapChainImageViews)
    {
        vkDestroyImageView(_device->_device, imageView, nullptr);
    }
    swapChainImageViews.clear();
    swapChainImages.clear();
    vkDestroySwapchainKHR(_device->_device, _swapchain, nullptr);
    width = newWidth;
    height = newHeight;
    extent = {newWidth, newHeight};
    VkSwapchainCreateInfoKHR swapchainCI = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = _device->_instance->_surface,
        .minImageCount = imageCount,
        .imageFormat = imageFormat,
        .imageColorSpace = _device->_gpu.surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        // .queueFamilyIndexCount;
        // .pQueueFamilyIndices;
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = _device->_gpu.presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };
    std::vector<uint32_t> queueFamilyIndices;
    queueFamilyIndices.push_back(_device->_gpu.graphicsFamily.value());
    queueFamilyIndices.push_back(_device->_gpu.presentFamily.value());

    if (_device->_gpu.graphicsFamily.value() != _device->_gpu.presentFamily.value())
    {
        swapchainCI.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCI.queueFamilyIndexCount = (uint32_t)queueFamilyIndices.size();
        swapchainCI.pQueueFamilyIndices = queueFamilyIndices.data();
    }
    else
    {
        swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    checkResult(vkCreateSwapchainKHR(_device->_device, &swapchainCI, nullptr, &_swapchain));
    loadImages();
    createImageViews();
}

void Swapchain::Release()
{
    for (auto imageView : swapChainImageViews)
    {
        vkDestroyImageView(_device->_device, imageView, nullptr);
    }
    swapChainImageViews.clear();
    swapChainImages.clear();
    vkDestroyRenderPass(_device->_device, _renderPass, nullptr);
    vkDestroyFence(_device->_device, _fence, nullptr);
    vkDestroySwapchainKHR(_device->_device, _swapchain, nullptr);
}