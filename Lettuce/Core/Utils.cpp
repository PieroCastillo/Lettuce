//
// Created by piero on 15/02/2024.
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <list>
#include <functional>
#include "Lettuce/Core/Utils.hpp"

using namespace Lettuce::Core;

void Lettuce::Core::checkResult(const VkResult result, std::string onSucessMessage)
{
    if (result == VK_SUCCESS || result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        // std::cout << onSucessMessage << std::endl;
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create vk object");
    }
}

char *Lettuce::Core::GetSurfaceExtensionNameByPlatform()
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

VkResult Lettuce::Core::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
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

void Lettuce::Core::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}