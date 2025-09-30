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
        enabledRecommendedFeatures.graphicsPipelineLibrary = true;

        addExt(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
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

    if (checkExtIfExists(VK_KHR_MAINTENANCE_5_EXTENSION_NAME))
    {
        enabledRecommendedFeatures.maintenance5 = true;

        addExt(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
        maintenance5Feature.maintenance5 = VK_TRUE;
        maintenance5Feature.pNext = next;
        next = (void *)&maintenance5Feature;
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

std::vector<VkDeviceQueueCreateInfo> Device::genQueuesInfos(VkPhysicalDevice pdevice)
{

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties2(_pdevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties2> queueFamilies(queueFamilyCount,
                                                        {VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2, 0, 0});
    vkGetPhysicalDeviceQueueFamilyProperties2(_pdevice, &queueFamilyCount, queueFamilies.data());

    std::vector<VkDeviceQueueCreateInfo> queuesCI;

    uint32_t index = 0;
    for (auto queueFamilyProps2 : queueFamilies)
    {
        VkQueueFamilyProperties queueFamilyProps = queueFamilyProps2.queueFamilyProperties;

        std::vector<float> priorities(queueFamilyProps.queueCount, 1.0f);

        queuesCI.push_back({
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .flags = queueFamilyProps.queueFlags,
            .queueFamilyIndex = index,
            .queueCount = queueFamilyProps.queueCount,
            .pQueuePriorities = priorities.data(),
        });

        index++;
    }

    return queuesCI;
}

void Device::getQueues(const std::vector<VkDeviceQueueCreateInfo> &queuesCI)
{
    uint32_t i = 0;
    for (auto queueCI : queuesCI)
    {
        QueueFamily queueFamily{};
        queueFamily.index = i;
        queueFamily.flags = queueCI.flags;

        for (uint32_t j = 0; j < queueCI.queueCount; j++)
        {
            VkQueue queue;
            vkGetDeviceQueue(GetHandle(), i, j, queue);

            queueFamily.queues.push_back(queue);
        }

        i++;
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

    // create queues
    auto queueCreateInfos = genQueuesInfos(gpu._pdevice);

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(gpu._pdevice, &features);

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

    checkResult(vkCreateDevice(_pdevice, &deviceCI, nullptr, GetHandlePtr()));

    VmaVulkanFunctions vulkanFunctions = {
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
        .vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements,
        .vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements,
    };

    VmaAllocatorCreateInfo allocatorI = {
        .physicalDevice = _pdevice,
        .device = GetHandle(),
        .pVulkanFunctions = &vulkanFunctions,
        .instance = _instance->_instance,
        .vulkanApiVersion = VK_API_VERSION_1_3,
    };
    checkResult(vmaCreateAllocator(&allocatorI, &allocator));

    // get all queues
    getQueues(queueCreateInfos);
}

Features Device::GetEnabledFeatures()
{
    return _enabledFeatures;
}

void Device::SubmitToQueue(VkQueueFlagBits queueFlag, const std::vector<VkSubmitInfo2> &infos)
{
    /*
    1. choose queue family
    2. choose queue
    3. submit to the queue that is possibly free

    also, this select the first graphics/video encode/decode that it found
    but if it's transfer or compute, select the last
    */

    uint32_t familyIndex = 0;
    if (queueFlag & VK_QUEUE_GRAPHICS_BIT != 0)
    {
        int i = 0;
        for (auto family : queueFamilies)
        {
            if (family.flags & VK_QUEUE_GRAPHICS_BIT != 0)
            {
                familyIndex = i;
                break;
            }

            i++;
        }
    }
    else if (queueFlag & VK_QUEUE_VIDEO_ENCODE_BIT_KHR != 0)
    {
        int i = 0;
        for (auto family : queueFamilies)
        {
            if (family.flags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR != 0)
            {
                familyIndex = i;
                break;
            }

            i++;
        }
    }
    else if (queueFlag & VK_QUEUE_VIDEO_DECODE_BIT_KHR != 0)
    {
        int i = 0;
        for (auto family : queueFamilies)
        {
            if (family.flags & VK_QUEUE_VIDEO_DECODE_BIT_KHR != 0)
            {
                familyIndex = i;
                break;
            }

            i++;
        }
    }
    else if (queueFlag & VK_QUEUE_COMPUTE_BIT != 0)
    {
        int i = 0;
        for (auto family : queueFamilies)
        {
            if (family.flags & (VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT) != 0)
            {
                familyIndex = i;
                break;
            }

            i++;
        }
    }
    else if (queueFlag & VK_QUEUE_TRANSFER_BIT != 0)
    {
        int i = 0;
        for (auto family : queueFamilies)
        {
            if (family.flags & (VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT) != 0)
            {
                familyIndex = i;
                break;
            }

            i++;
        }
    }
    QueueFamily family = queueFamilies[(int)familyIndex];

    VkQueue currentQueue = family.queues[(int)family.currentQueueIndex];

    checkResult(vkQueueSubmit2(currentQueue, (uint32_t)infos.size(), infos.data(), VK_NULL_HANDLE));

    queueFamilies[(int)familyIndex].currentQueueIndex = (family.currentQueueIndex + 1) % family.queues.size();
}

void Device::Wait()
{
    checkResult(vkDeviceWaitIdle(GetHandle()));
}

void Device::Release()
{
    vmaDestroyAllocator(allocator);
    vkDestroyDevice(GetHandle(), nullptr);
}