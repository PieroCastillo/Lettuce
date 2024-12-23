//
// Created by piero on 11/02/2024.
//
#include "Lettuce/Core/common.hpp"
#include <algorithm>
#include <iostream>
#include <optional>
#include <vector>
#include "Lettuce/Core/GPU.hpp"

using namespace Lettuce::Core;

void GPU::Create(VkSurfaceKHR &surface, VkPhysicalDevice &device)
{
    _surface = surface;
    _pdevice = device;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    deviceType = deviceProperties.deviceType;
    geometryShaderPresent = deviceFeatures.geometryShader;
    bufferImageGranularity = deviceProperties.limits.bufferImageGranularity;
    deviceName = std::string(deviceProperties.deviceName);
    loadQueuesFamilies();
    checkSurfaceCapabilities();
    std::cout << "gpu ptr: " << _pdevice << std::endl;
}

bool GPU::GraphicsCapable()
{
    return graphicsFamily.has_value() && presentFamily.has_value() && !formats.empty() && !presentModes.empty();
}

VkFormat GPU::GetDepthFormat()
{
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(_pdevice, VK_FORMAT_D24_UNORM_S8_UINT, &properties);
    bool d24s8_support = (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    if (d24s8_support)
    {
        return VK_FORMAT_D24_UNORM_S8_UINT;
    }
    return VK_FORMAT_D32_SFLOAT_S8_UINT;
}

uint32_t GPU::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_pdevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    return 0;
}

void GPU::checkSurfaceCapabilities()
{
    if (_surface == VK_NULL_HANDLE)
        return;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_pdevice, _surface, &capabilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(_pdevice, _surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(_pdevice, _surface, &formatCount, formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(_pdevice, _surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(_pdevice, _surface, &presentModeCount, presentModes.data());
    }

    if (!GraphicsCapable())
        return;

    // selects the better format
    for (const auto &availableFormat : formats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            surfaceFormat = availableFormat;
        }
    }
    surfaceFormat = formats[0];

    // selects the presentations mode
    for (const auto &availablePresentMode : presentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            presentMode = availablePresentMode;
        }
    }
    presentMode = VK_PRESENT_MODE_FIFO_KHR;
}

void GPU::loadQueuesFamilies()
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(_pdevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(_pdevice, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        if (!graphicsFamily.has_value() && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
        {
            graphicsFamily = i;
            presentFamily = i;
        }

        if (!computeFamily.has_value() && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))
        {
            computeFamily = i;
        }

        if (GraphicsCapable())
            break;

        i++;
    }
}

VkFormat GPU::findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(_pdevice, format, &props);
        if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}