//
// Created by piero on 14/02/2024.
//
#pragma once
#include <iostream>
#include <string>
#include <functional>
#include <tuple>
#include <algorithm>
#include "Device.hpp"
#include "IReleasable.hpp"
#include "Semaphore.hpp"

namespace Lettuce::Core
{
    class Swapchain : public IReleasable
    {
    private:
        std::function<std::tuple<uint32_t, uint32_t>(void)> _func;
        std::function<void(void)> _postFunc;
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;

        uint32_t imageCount;

        void loadImages();
        void createImageViews();

    public:
        std::shared_ptr<Device> _device;
        VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
        VkRenderPass _renderPass = VK_NULL_HANDLE;
        // Texture _depthImage;
        // TextureView _depthImageView;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;
        uint32_t index;
        uint32_t width;
        uint32_t height;
        VkFormat imageFormat;
        // VkFormat depthFormat;
        VkExtent2D extent;
        VkFence _fence;

        Swapchain(const std::shared_ptr<Device> &device, uint32_t initialWidth, uint32_t initialHeight);
        void Release();

        void SetResizeFunc(std::function<std::tuple<uint32_t, uint32_t>(void)> call, std::function<void(void)> postFunc);

        void AcquireNextImage();

        void Present();

        void Resize(uint32_t newWidth, uint32_t newHeight);

        void Wait();

    };
}