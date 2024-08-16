//
// Created by piero on 15/02/2024.
//
#include <iostream>
#include <stdexcept>
#include <vector>
#include <list>
#include <functional>
#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#include <ddkernel.h>
#include <vulkan/vulkan_win32.h>
#endif
#if defined(__linux__) || defined(__unix__)
#define VK_USE_PLATFORM_XLIB_KHR
// #include <vulkan/vulkan_wayland.h>
#endif
#if defined(__APPLE__)
#define VK_USE_PLATFORM_MACOS_MVK
// #include <vulkan/vulkan_macos.h>
#endif
#include <volk.h>
#include "Lettuce/Core/Utils.hpp"

using namespace Lettuce::Core;

void checkResult(const VkResult result, std::string onSucessMessage = "created successfully!")
{
    if (result == VK_SUCCESS)
    {
        // std::cout << onSucessMessage << std::endl;
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create vk object");
    }
}

char *GetSurfaceExtensionNameByPlatform()
{
    char *surfaceExtensionName;
#if defined(_WIN32)
    return (char *)VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#endif
#if defined(__linux__)
    return (char *)VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
#endif
#if defined(__APPLE__)
    return (char *)VK_MVK_MACOS_SURFACE_EXTENSION_NAME;
#endif
    return (char *)"";
}

template <typename T1, typename T2>
VkResult CreateVkSurface(VkInstance instance, T1 window, T2 process, VkSurfaceKHR &surface, const VkAllocationCallbacks *allocator)
{
#if defined(_WIN32)
    VkWin32SurfaceCreateInfoKHR surfaceCI = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = (static_cast<HINSTANCE>(process)),
        .hwnd = (static_cast<HWND>(window))};
    return vkCreateWin32SurfaceKHR(instance, &surfaceCI, allocator, &surface);
#endif
#if defined(__linux__)
    return VK_ERROR_INITIALIZATION_FAILED;
#endif
#if defined(__APPLE__)
    return VK_ERROR_INITIALIZATION_FAILED;
#endif
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}
