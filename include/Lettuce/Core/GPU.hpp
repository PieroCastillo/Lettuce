//
// Created by piero on 11/02/2024.
//
#pragma once
#include <algorithm>
#include <iostream>
#include <optional>
#include <vector>
#include <volk.h>

namespace Lettuce::Core
{
    class GPU
    {
    public:
        VkSurfaceKHR _surface;
        VkPhysicalDevice _pdevice;
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        VkPhysicalDeviceType deviceType;

        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
        VkSurfaceFormatKHR surfaceFormat;
        VkPresentModeKHR presentMode;

        std::string deviceName;
        bool geometryShaderPresent = false;
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::vector<char *> availableExtensionsNames;

        void Create(VkSurfaceKHR &surface, VkPhysicalDevice &device);

        bool GraphicsCapable();

        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    private:
        void checkSurfaceCapabilities();

        void loadQueuesFamilies();
    };
}