//
// Created by piero on 11/02/2024.
//
#include "Lettuce/Core/common.hpp"
#ifdef VK_USE_PLATFORM_WIN32_KHR 
#include <ddkernel.h>
#endif
#include <iostream>
#include <vector>
#include <cstdint>
#include <list>
#include "Lettuce/Core/Instance.hpp"
#include "Lettuce/Core/Utils.hpp"

using namespace Lettuce::Core;

VKAPI_ATTR VkBool32 VKAPI_CALL Instance::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    // false positives
    switch (pCallbackData->messageIdNumber)
    {
    // case 1219306694:
    //     break;
    default:
        std::cerr << "validation layer says: " << "[" << pCallbackData->messageIdNumber << "] " << pCallbackData->pMessage << std::endl;
        break;
    }

    return VK_FALSE;
}

void Instance::listExtensions()
{
    uint32_t availableExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());
    for (auto ext : availableExtensions)
    {
        availableExtensionsNames.push_back(ext.extensionName);
        std::cout << "available instance extensions :" << ext.extensionName << std::endl;
    }
}

void Instance::listLayers()
{
    uint32_t availableLayerCount = 0;
    vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(availableLayerCount);
    vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.data());
    for (auto layer : availableLayers)
    {
        availableLayersNames.push_back(layer.layerName);
        std::cout << "available instance layer : " << layer.layerName << std::endl;
    }
}

void Instance::loadPlatformAndFeatures()
{
    auto name = GetSurfaceExtensionNameByPlatform();
    std::cout << "surface ext name: " << name << std::endl;
    // all needed vk extensions here
    requestedExtensionsNames.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
    if (_debug)
    {
        requestedExtensionsNames.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    requestedExtensionsNames.emplace_back(name);
}

void Instance::Create(std::string appName, Version appVersion, std::vector<char *> requestedExtensions)
{
    for (auto ext : requestedExtensions)
    {
        requestedExtensionsNames.emplace_back(ext);
    }
    if (_debug)
    {
        requestedLayersNames.emplace_back("VK_LAYER_KHRONOS_validation");
    }

    // Lettuce::Core::VulkanFunctions vf;
    // auto loaded = vf.load_vulkan_library();

    // if (!loaded)
    // {
    //     std::cout << "cannot load vulkan library" << std::endl;
    //     return;
    // }
    checkResult(volkInitialize());
    VkDebugUtilsMessengerCreateInfoEXT debugUtilsCI =
        {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT,
            .pfnUserCallback = debugCallback,
            .pUserData = nullptr, // Optional
        };

    VkApplicationInfo appI = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = appName.c_str(),
        .applicationVersion = VK_MAKE_API_VERSION(appVersion.variant, appVersion.major, appVersion.minor, appVersion.patch),
        .pEngineName = "Lettuce Rendering Engine",
        .engineVersion = VK_MAKE_API_VERSION(VARIANT_VERSION, MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION),
        .apiVersion = VK_MAKE_VERSION(1, 3, 0),
    };
    listExtensions();
    loadPlatformAndFeatures();
    listLayers();

    VkInstanceCreateInfo instanceCI = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appI,
        .ppEnabledLayerNames = nullptr,
        .ppEnabledExtensionNames = nullptr,
    };

    if (requestedLayersNames.size() > 0)
    {
        instanceCI.enabledLayerCount = (uint32_t)requestedLayersNames.size();
        instanceCI.ppEnabledLayerNames = requestedLayersNames.data();
    }

    if (requestedExtensionsNames.size() > 0)
    {
        instanceCI.enabledExtensionCount = (uint32_t)requestedExtensionsNames.size();
        instanceCI.ppEnabledExtensionNames = requestedExtensionsNames.data();
    }

    if (_debug)
    {
        instanceCI.pNext = &debugUtilsCI;
    }

    checkResult(vkCreateInstance(&instanceCI, nullptr, &_instance), "instance created successfully");
    volkLoadInstance(_instance);
    if (_debug)
    {
        checkResult(CreateDebugUtilsMessengerEXT(_instance, &debugUtilsCI, nullptr, &debugMessenger), "debug messenger created successfully");
    }
}

std::list<GPU> Instance::getGPUs()
{
    uint32_t physicalDevicesCount = 0;
    vkEnumeratePhysicalDevices(_instance, &physicalDevicesCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
    vkEnumeratePhysicalDevices(_instance, &physicalDevicesCount, physicalDevices.data());

    std::list<GPU> gpus(physicalDevicesCount - 1);
    for (auto dev : physicalDevices)
    {
        GPU gpu;
        gpu.Create(_surface, dev);
        gpus.push_front(gpu);
    }
    return gpus;
}

bool Instance::IsSurfaceCreated()
{
    return isSurfaceCreated;
}

void Instance::Destroy()
{
    if (isSurfaceCreated)
    {
        vkDestroySurfaceKHR(_instance, _surface, nullptr);
    }
    if (_debug)
    {
        DestroyDebugUtilsMessengerEXT(_instance, debugMessenger, nullptr);
    }
    volkFinalize();
    vkDestroyInstance(_instance, nullptr);
}