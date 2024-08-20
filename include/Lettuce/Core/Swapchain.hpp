//
// Created by piero on 14/02/2024.
//
#pragma once
#include <iostream>
#include <string>
#include <algorithm>
#include <volk.h>
#include "Device.hpp"
#include "Semaphore.hpp"

namespace Lettuce::Core
{
    class Swapchain
    {
    private:
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;

        uint32_t imageCount;

        void loadImages();
        void createImageViews();

    public:
        Device _device;
        VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;
        uint32_t index;
        uint32_t width;
        uint32_t height;
        VkFormat imageFormat;
        VkExtent2D extent;

        void Create(Device &device, uint32_t initialWidth, uint32_t initialHeight);

        void AcquireNextImage(BSemaphore acquireImageSemaphore);

        void Present(BSemaphore renderSemaphore);

        void Destroy();
    };
}