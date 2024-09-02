//
// Created by piero on 14/02/2024.
//
#pragma once
#include <iostream>
#include <string>
#include <algorithm>
#include "Device.hpp"
#include "Semaphore.hpp"
#include "Texture.hpp"
#include "TextureView.hpp"

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
        void createDepthImage();
        void createRenderPass();
        void createFramebuffers();

    public:
        Device _device;
        VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
        VkRenderPass _renderPass = VK_NULL_HANDLE;
        Texture _depthImage;
        TextureView _depthImageView;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;
        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkFramebuffer> framebuffers;
        uint32_t index;
        uint32_t width;
        uint32_t height;
        VkFormat imageFormat;
        VkFormat depthFormat;
        VkExtent2D extent;

        void Create(Device &device, uint32_t initialWidth, uint32_t initialHeight);

        void AcquireNextImage(BSemaphore acquireImageSemaphore);

        void Present();

        void Resize(uint32_t newWidth, uint32_t newHeight);

        void Wait();

        void Destroy();
    };
}