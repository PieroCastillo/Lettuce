//
// Created by piero on 14/02/2024.
//
module;

#include <iostream>
#include <string>
#define VOLK_IMPLEMENTATION
#include <volk.h>

export module Lettuce:Swapchain;

import :Device;
import :Instance;
import :SynchronizationStructure;
import :GPU;

export namespace Lettuce::Core
{
    class Swapchain
    {
    private:
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;

        uint32_t imageCount;
        VkExtent2D extent;

        void loadImages()
        {
            vkGetSwapchainImagesKHR(_device._device, _swapchain, &imageCount, nullptr);
            swapChainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(_device._device, _swapchain, &imageCount, swapChainImages.data());
        }

        void createImageViews()
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
                    .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
                checkResult(vkCreateImageView(_device._device, &imageViewCI, nullptr, &swapChainImageViews[i]), "imageView " + std::to_string(i));
            }
        }

    public:
        Device _device;
        SynchronizationStructure _sync;
        VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;
        uint32_t index;
        uint32_t width;
        uint32_t height;
        VkFormat imageFormat;

        void Create(Device &device, SynchronizationStructure &sync, uint32_t initialWidth, uint32_t initialHeight)
        {
            _device = device;
            _sync = sync;
            auto capabilities = _device._gpu.capabilities;
            imageCount = capabilities.minImageCount + 1;
            if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
            {
                imageCount = capabilities.maxImageCount;
            }
            imageFormat = _device._gpu.surfaceFormat.format;
            width = initialWidth;
            height = initialHeight;
            extent = {initialWidth, initialHeight};
            VkSwapchainCreateInfoKHR swapchainCI = {
                .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .surface = _device._instance._surface,
                .minImageCount = imageCount,
                .imageFormat = imageFormat,
                .imageColorSpace = _device._gpu.surfaceFormat.colorSpace,
                .imageExtent = extent,
                .imageArrayLayers = 1,
                .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                // .imageSharingMode;
                // .queueFamilyIndexCount;
                // .pQueueFamilyIndices;
                .preTransform = capabilities.currentTransform,
                .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                .presentMode = _device._gpu.presentMode,
                .clipped = VK_TRUE,
                .oldSwapchain = VK_NULL_HANDLE};

            uint32_t queueFamilyIndices[] = {_device._gpu.graphicsFamily.value(), _device._gpu.presentFamily.value()};

            if (_device._gpu.graphicsFamily.value() != _device._gpu.presentFamily.value())
            {
                swapchainCI.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                swapchainCI.queueFamilyIndexCount = 2;
                swapchainCI.pQueueFamilyIndices = queueFamilyIndices;
            }
            else
            {
                swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            }

            checkResult(vkCreateSwapchainKHR(_device._device, &swapchainCI, nullptr, &_swapchain), "swapchain created successfully");

            loadImages();
            createImageViews();
        }

        void AcquireNextImage(int acquireImageSemaphoreIndex)
        {
            vkAcquireNextImageKHR(_device._device, _swapchain, std::numeric_limits<uint64_t>::max(), _sync.semaphores[acquireImageSemaphoreIndex], nullptr, &index);
        }

        void Present(int renderSemaphoreIndex)
        {
            VkPresentInfoKHR presentI = {
                .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &_sync.semaphores[renderSemaphoreIndex],
                .swapchainCount = 1,
                .pSwapchains = &_swapchain,
                .pImageIndices = &index
            };

            vkQueuePresentKHR(_device._presentQueue, &presentI);
            // vkQueueWaitIdle(_device._presentQueue);
        }

        void Destroy()
        {
            for (auto imageView : swapChainImageViews)
            {
                vkDestroyImageView(_device._device, imageView, nullptr);
            }
            vkDestroySwapchainKHR(_device._device, _swapchain, nullptr);
        }
    };

}