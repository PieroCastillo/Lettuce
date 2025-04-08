//
// Created by piero on 14/02/2024.
//
#include "Lettuce/Core/Common.hpp"

#include <iostream>
#include <set>
#include <vector>
#include <algorithm>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Utils.hpp"

using namespace Lettuce::Core;

void Device::addExt(const std::string &extName)
{
    int i = 0;
    for (auto ext : availableExtensionsNames)
    {
        if (ext == extName)
        {
            break;
        }
        i++;
    }

    // use the same memory of available extensions names list
    requestedExtensionsNames.push_back(availableExtensionsNames[i].c_str());
}

bool Device::checkExtIfExists(const std::string &extName)
{
    int val = std::count(availableExtensionsNames.begin(),
                         availableExtensionsNames.end(),
                         extName);

    return val > 0 ? true : false;
}

void Device::addRequiredFeatures()
{
    addExt(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    addExt(VK_EXT_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_EXTENSION_NAME);

    gpuFeatures12.scalarBlockLayout = VK_TRUE; // enables  GL_EXT_scalar_block_layout
    gpuFeatures12.bufferDeviceAddress = VK_TRUE;
    gpuFeatures12.drawIndirectCount = VK_TRUE;
    // enables descriptor indexing
    gpuFeatures12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    gpuFeatures12.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
    gpuFeatures12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    gpuFeatures12.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
    gpuFeatures12.descriptorBindingPartiallyBound = VK_TRUE;
    gpuFeatures12.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
    gpuFeatures12.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
    gpuFeatures12.descriptorIndexing = VK_TRUE;
    gpuFeatures12.runtimeDescriptorArray = VK_TRUE;

    gpuFeatures12.timelineSemaphore = VK_TRUE;

    gpuFeatures13.synchronization2 = VK_TRUE;
    gpuFeatures13.dynamicRendering = VK_TRUE;

    descriptorBufferFeature.descriptorBuffer = VK_TRUE;

    dynamicRenderingUnusedAttachmentsFeature.dynamicRenderingUnusedAttachments = VK_TRUE;
    next = (void *)&dynamicRenderingUnusedAttachmentsFeature;
}

void Device::addRecommendedFeatures()
{
    if (checkExtIfExists(VK_EXT_SHADER_OBJECT_EXTENSION_NAME))
    {
        enabledRecommendedFeatures.shaderObject = true;

        addExt(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
        shaderObjectFeature.shaderObject = VK_TRUE;
        shaderObjectFeature.pNext = next;
        next = (void *)&shaderObjectFeature;
    }

    if (checkExtIfExists(VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME))
    {
        enabledRecommendedFeatures.deviceGeneratedCommands = true;

        addExt(VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME);
        deviceGeneratedCommandsFeature.deviceGeneratedCommands = VK_TRUE;
        deviceGeneratedCommandsFeature.dynamicGeneratedPipelineLayout = VK_TRUE;
        deviceGeneratedCommandsFeature.pNext = next;
        next = (void *)&deviceGeneratedCommandsFeature;
    }

    if (checkExtIfExists(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME))
    {
        enabledRecommendedFeatures.graphicPipelineLibrary = true;

        addExt(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
        graphicsPipelineLibraryFeature.graphicsPipelineLibrary = VK_TRUE;
        graphicsPipelineLibraryFeature.pNext = next;
        next = (void *)&graphicsPipelineLibraryFeature;
    }

    if (checkExtIfExists(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME))
    {
        enabledRecommendedFeatures.dynamicRenderingLocalRead = true;

        addExt(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME);
        dynamicRenderingLocalReadFeature.dynamicRenderingLocalRead = VK_TRUE;
        dynamicRenderingLocalReadFeature.pNext = next;
        next = (void *)&dynamicRenderingLocalReadFeature;
    }
}

void Device::addOptionalFeatures()
{
    /*
        - Video Extensions
        - Execution Graphs
        - Ray Tracing
        - Mesh Shading
        - Fragment Shading Rate
    */
    if (_features.MeshShading && checkExtIfExists(VK_EXT_MESH_SHADER_EXTENSION_NAME))
    {
        addExt(VK_EXT_MESH_SHADER_EXTENSION_NAME);
        meshShaderFeature.taskShader = VK_TRUE;
        meshShaderFeature.meshShader = VK_TRUE;
        _enabledFeatures.MeshShading = true;
        meshShaderFeature.pNext = next;
        next = (void *)&meshShaderFeature;
    }
    if (_features.FragmentShadingRate && checkExtIfExists(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME))
    {
        addExt(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
        fragmentShadingRateFeature.pipelineFragmentShadingRate = VK_TRUE;
        fragmentShadingRateFeature.primitiveFragmentShadingRate = VK_TRUE;
        fragmentShadingRateFeature.attachmentFragmentShadingRate = VK_TRUE;
        _enabledFeatures.FragmentShadingRate = true;
        fragmentShadingRateFeature.pNext = next;
        next = (void *)&fragmentShadingRateFeature;
    }
}

void Device::listExtensions()
{
    uint32_t availableExtensionCount = 0;
    // std::cout << *_pdevice << std::endl;
    vkEnumerateDeviceExtensionProperties(_pdevice, nullptr, &availableExtensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
    vkEnumerateDeviceExtensionProperties(_pdevice, nullptr, &availableExtensionCount, availableExtensions.data());
    for (auto ext : availableExtensions)
    {
        availableExtensionsNames.push_back(std::string(ext.extensionName));
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
    if (_instance->_debug)
    {
        requestedLayersNames.push_back("VK_LAYER_KHRONOS_validation");
    }

    if (_instance->IsSurfaceCreated())
    {
        requestedExtensionsNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }
}

Device::Device(const std::shared_ptr<Instance> &instance, GPU &gpu, Features gpuFeatures, uint32_t graphicsQueuesCount) : _pdevice(gpu._pdevice),
                                                                                                                          _instance(instance),
                                                                                                                          _gpu(gpu),
                                                                                                                          _features(gpuFeatures)
{
    listExtensions();
    listLayers();
    loadExtensionsLayersAndFeatures();

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

    if (gpu.computeFamily.value() != gpu.graphicsFamily.value())
    {
        VkDeviceQueueCreateInfo computeQueueCI = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = gpu.computeFamily.value(),
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        };
        queueCreateInfos.push_back(computeQueueCI);
    }

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(_pdevice, &features);

    // here we enable device features, like Buffer Device Address, Timeline Semaphores, etc

    addRequiredFeatures();
    addRecommendedFeatures();
    addOptionalFeatures();

    VkDeviceCreateInfo deviceCI = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = next,
        .queueCreateInfoCount = (uint32_t)queueCreateInfos.size(),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = (uint32_t)requestedExtensionsNames.size(),
        .ppEnabledExtensionNames = requestedExtensionsNames.data(),
        .pEnabledFeatures = &features,
    };

    checkResult(vkCreateDevice(_pdevice, &deviceCI, nullptr, &_device));

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
        .instance = _instance->_instance,
        .vulkanApiVersion = VK_API_VERSION_1_3,
    };
    checkResult(vmaCreateAllocator(&allocatorI, &allocator));

    // get all queues
    _graphicsQueues.resize(graphicsQueuesCount);
    for (int i = 0; i < graphicsQueuesCount; i++)
    {
        vkGetDeviceQueue(_device, gpu.graphicsFamily.value(), i, &(_graphicsQueues.at(i)));
    }
    vkGetDeviceQueue(_device, gpu.presentFamily.value(), 0, &_presentQueue);
    vkGetDeviceQueue(_device, gpu.computeFamily.value(), 0, &_computeQueue);
}

Features Device::GetEnabledFeatures()
{
    return _enabledFeatures;
}

void Device::Wait()
{
    checkResult(vkDeviceWaitIdle(_device));
}

void Device::Release()
{
    vmaDestroyAllocator(allocator);
    vkDestroyDevice(_device, nullptr);
}