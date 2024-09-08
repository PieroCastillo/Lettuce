//
// Created by piero on 14/02/2024.
//
#include "Lettuce/Core/common.hpp"

#include <iostream>
#include <set>
#include <vector>
#include <algorithm>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Utils.hpp"

using namespace Lettuce::Core;

void Device::listExtensions()
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

void Device::listLayers()
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

void Device::loadExtensionsLayersAndFeatures()
{
    if (_instance._debug)
    {
        requestedLayersNames.emplace_back("VK_LAYER_KHRONOS_validation");
    }

    if (_instance.IsSurfaceCreated())
    {
        requestedExtensionsNames.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }
    // requestedExtensionsNames.emplace_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    // requestedExtensionsNames.emplace_back(VK_KHR_PRESENT_WAIT_EXTENSION_NAME);
    // requestedExtensionsNames.emplace_back(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
    // requestedExtensionsNames.emplace_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
    //  requestedExtensionsNames.emplace_back(VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    // requestedExtensionsNames.emplace_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
}

void Device::Create(Instance &instance, GPU &gpu, std::vector<char *> requestedExtensions, uint32_t graphicsQueuesCount)
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

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos; // here we store all queues CI
    std::vector<float> queuePriorities(graphicsQueuesCount, 1.0f);
    float queuePriority = 1.0f;

    // create  queues for graphics
    VkDeviceQueueCreateInfo graphicsQueuesCI = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = gpu.graphicsFamily.value(),
        .queueCount = graphicsQueuesCount,
        .pQueuePriorities = queuePriorities.data(),
    };
    queueCreateInfos.push_back(graphicsQueuesCI);

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(_pdevice, &features);

    // here we enable device features, like Buffer Device Address, Timeline Semaphores, etc

    // VkPhysicalDevicePresentWaitFeaturesKHR presentWaitFeature = {
    //     .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR,
    //     .pNext = &sync2Feature,
    //     .presentWait = VK_TRUE,
    // };

    // VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragmentShadingRateFeature = {
    //     .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR,
    //     .pNext = &bufferDeviceAddressFeature,
    //     .pipelineFragmentShadingRate = VK_TRUE,
    //     .primitiveFragmentShadingRate = VK_TRUE,
    //     .attachmentFragmentShadingRate = VK_TRUE,
    // };

    VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeature = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
        // .pNext = &timelineSemaphoreFeature,
        .taskShader = VK_TRUE,
        .meshShader = VK_TRUE,
    };

    // always available in vulkan 1.2
    VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeature = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
        //.pNext = &meshShaderFeature,
        .bufferDeviceAddress = VK_TRUE,
        .bufferDeviceAddressCaptureReplay = VK_FALSE,
    };

    VkPhysicalDeviceDescriptorIndexingFeatures deviceDescriptorIndexingFeature = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
        .pNext = &bufferDeviceAddressFeature,
        .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
        .descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE,
        .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
        .descriptorBindingUpdateUnusedWhilePending = VK_TRUE,
        .descriptorBindingPartiallyBound = VK_TRUE,
        .descriptorBindingVariableDescriptorCount = VK_TRUE,
        .runtimeDescriptorArray = VK_TRUE,
    };

    // always available in vulkan 1.3
    VkPhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreFeature = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES,
        .pNext = &deviceDescriptorIndexingFeature,
        .timelineSemaphore = VK_TRUE,
    };

    VkPhysicalDeviceSynchronization2Features sync2Feature = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
        .pNext = &timelineSemaphoreFeature,
        .synchronization2 = VK_TRUE,
    };

    VkDeviceCreateInfo deviceCI = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &sync2Feature,
        .queueCreateInfoCount = (uint32_t)queueCreateInfos.size(),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = (uint32_t)requestedExtensionsNames.size(),
        .ppEnabledExtensionNames = requestedExtensionsNames.data(),
        .pEnabledFeatures = &features,
    };

    checkResult(vkCreateDevice(_pdevice, &deviceCI, nullptr, &_device), "device created successfully");

    VmaVulkanFunctions vulkanFunctions = {
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
        .vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements,
        .vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements,
    };

    VmaAllocatorCreateInfo allocatorI = {
        .physicalDevice = _pdevice,
        .device = _device,
        .pVulkanFunctions = &vulkanFunctions,
        .instance = _instance._instance,
        .vulkanApiVersion = VK_API_VERSION_1_3,
    };
    checkResult(vmaCreateAllocator(&allocatorI, &allocator), "allocator created successfully");

    // get all queues
    _graphicsQueues.resize(graphicsQueuesCount);
    for (int i = 0; i < graphicsQueuesCount; i++)
    {
        vkGetDeviceQueue(_device, gpu.graphicsFamily.value(), i, &(_graphicsQueues.at(i)));
    }
    vkGetDeviceQueue(_device, gpu.presentFamily.value(), 0, &_presentQueue);
}

void Device::Wait()
{
    checkResult(vkDeviceWaitIdle(_device));
}

void Device::Destroy()
{
    vmaDestroyAllocator(allocator);
    vkDestroyDevice(_device, nullptr);
}