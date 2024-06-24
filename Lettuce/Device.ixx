//
// Created by piero on 14/02/2024.
//
module;

#include <iostream>
#include <set>
#include <vector>
#define VOLK_IMPLEMENTATION
#include <volk.h>

export module Lettuce:Device;

import :Instance;
import :GPU;
import :Utils;

export namespace Lettuce::Core
{
    class Device
    {
    private:
        std::vector<char *> availableExtensionsNames;
        std::vector<char *> availableLayersNames;
        std::vector<const char *> requestedExtensionsNames;
        std::vector<const char *> requestedLayersNames;

        void listExtensions()
        {
            uint32_t availableExtensionCount = 0;
            // std::cout << *_pdevice << std::endl;
            vkEnumerateDeviceExtensionProperties(_pdevice, nullptr, &availableExtensionCount, nullptr);
            std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
            vkEnumerateDeviceExtensionProperties(_pdevice, nullptr, &availableExtensionCount, availableExtensions.data());
            for (auto ext : availableExtensions)
            {
                availableExtensionsNames.push_back(ext.extensionName);
                std::cout << "available device extensions :" << ext.extensionName << std::endl;
            }
        }

        void listLayers()
        {
            uint32_t availableLayerCount = 0;
            vkEnumerateDeviceLayerProperties(_pdevice, &availableLayerCount, nullptr);
            std::vector<VkLayerProperties> availableLayers(availableLayerCount);
            vkEnumerateDeviceLayerProperties(_pdevice, &availableLayerCount, availableLayers.data());
            for (auto layer : availableLayers)
            {
                availableLayersNames.push_back(layer.layerName);
                std::cout << "available device layer : " << layer.layerName << std::endl;
            }
        }

        void loadExtensionsLayersAndFeatures()
        {
            if (_instance._debug)
            {
                requestedLayersNames.emplace_back("VK_LAYER_KHRONOS_validation");
            }

            if (_instance.IsSurfaceCreated())
            {
                requestedExtensionsNames.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
                requestedExtensionsNames.emplace_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
            }
            // vulkan 1.3 add dynamic rendering and synchronization2 to the core
        }

    public:
        Instance _instance;
        VkPhysicalDevice _pdevice;
        VkDevice _device;
        VkQueue _graphicsQueue;
        VkQueue _presentQueue;
        GPU _gpu;

        void Create(Instance &instance, GPU &gpu, std::vector<char *> requestedExtensions)
        {
            _pdevice = gpu._pdevice;
            _instance = instance;
            _gpu = gpu;
            listExtensions();
            listLayers();
            loadExtensionsLayersAndFeatures();

            for (auto ext : requestedExtensions)
            {
                requestedExtensionsNames.emplace_back(ext);
            }

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            std::set<uint32_t> uniqueQueueFamilies = {gpu.graphicsFamily.value(), gpu.presentFamily.value()};
            float queuePriority = 1.0f;
            for (uint32_t queueFamily : uniqueQueueFamilies)
            {
                VkDeviceQueueCreateInfo queueCreateInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .queueFamilyIndex = queueFamily,
                    .queueCount = 1,
                    .pQueuePriorities = &queuePriority};
                queueCreateInfos.push_back(queueCreateInfo);
            }

            VkPhysicalDeviceFeatures features;
            vkGetPhysicalDeviceFeatures(_pdevice, &features);

            // VkPhysicalDeviceSynchronization2Features sync2Feature ={
            //     .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
            //     .synchronization2 = VK_TRUE
            // };

            // enables dynamic rendering
            VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeature = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
                //.pNext = &sync2Feature,
                .dynamicRendering = VK_TRUE,
            };

            VkDeviceCreateInfo deviceCI = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                .pNext = &dynamicRenderingFeature,
                .queueCreateInfoCount = (uint32_t)queueCreateInfos.size(),
                .pQueueCreateInfos = queueCreateInfos.data(),
                .enabledLayerCount = 0,
                .ppEnabledLayerNames = nullptr,
                .enabledExtensionCount = (uint32_t)requestedExtensionsNames.size(),
                .ppEnabledExtensionNames = requestedExtensionsNames.data(),
                .pEnabledFeatures = &features};

            checkResult(vkCreateDevice(_pdevice, &deviceCI, nullptr, &_device), "device created successfully");

            vkGetDeviceQueue(_device, gpu.graphicsFamily.value(), 0, &_graphicsQueue);
            vkGetDeviceQueue(_device, gpu.presentFamily.value(), 0, &_presentQueue);
        }

        void Wait()
        {
            vkDeviceWaitIdle(_device);
        }

        void Destroy()
        {
            vkDestroyDevice(_device, nullptr);
        }
    };
}