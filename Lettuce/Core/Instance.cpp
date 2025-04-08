//
// Created by piero on 11/02/2024.
//
#include "Lettuce/Core/Common.hpp"
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
    requestedExtensionsNames.emplace_back(name);
}

Instance::Instance(std::string appName, Version appVersion, std::vector<char *> requestedExtensions)
{
    for (auto ext : requestedExtensions)
    {
        requestedExtensionsNames.emplace_back(ext);
    }

    // Lettuce::Core::VulkanFunctions vf;
    // auto loaded = vf.load_vulkan_library();

    // if (!loaded)
    // {
    //     std::cout << "cannot load vulkan library" << std::endl;
    //     return;
    // }
    checkResult(volkInitialize());

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

    checkResult(vkCreateInstance(&instanceCI, nullptr, &_instance));
    volkLoadInstance(_instance);
}

void Instance::Release()
{
    if (isSurfaceCreated)
    {
        vkDestroySurfaceKHR(_instance, _surface, nullptr);
    }
    vkDestroyInstance(_instance, nullptr);
    volkFinalize();
}

std::vector<GPU> Instance::getGPUs()
{
    uint32_t physicalDevicesCount = 0;
    vkEnumeratePhysicalDevices(_instance, &physicalDevicesCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
    vkEnumeratePhysicalDevices(_instance, &physicalDevicesCount, physicalDevices.data());

    std::vector<GPU> gpus;
    gpus.reserve(physicalDevicesCount - 1);
    for (auto dev : physicalDevices)
    {
        auto gpu = GPU(_surface, dev);
        gpus.push_back(gpu);
    }
    return gpus;
}

bool Instance::IsSurfaceCreated()
{
    return isSurfaceCreated;
}