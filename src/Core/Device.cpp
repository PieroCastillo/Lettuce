// standard headers
#include <memory>
#include <variant>
#include <expected>
#include <vector>
#include <future>
#include <unordered_map>
#include <array>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/CommandList.hpp"
#include "Lettuce/Core/GPU.hpp"

using namespace Lettuce::Core;

template <typename T>
inline bool exists(const std::vector<T>& vec, const T& elem)
{
    return std::find(vec.begin(), vec.end(), elem) != vec.end();
}


Device::Device()
{
    setupInstance();
}

void Device::Build()
{
    setupDevice();
    getQueues();
}

void Device::Release()
{
    vkDestroyDevice(m_device, nullptr);
    vkDestroyInstance(m_instance, nullptr);
    volkFinalize();
}

void Device::setupInstance()
{
    volkInitialize();
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Lettuce Application",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "Lettuce Rendering Library",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_3,
    };

    std::array<const char*, 2> instanceExtensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef WIN32_
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
    };

    VkInstanceCreateInfo instanceCI = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = (uint32_t)instanceExtensions.size(), // Add required extensions here
        .ppEnabledExtensionNames = instanceExtensions.data(),
        .enabledLayerCount = 0, // Add required layers here
        .ppEnabledLayerNames = nullptr,
    };
    // TODO: Add validation to vulkan functions
    handleResult(vkCreateInstance(&instanceCI, nullptr, &m_instance));
    volkLoadInstanceOnly(m_instance);
}

void Device::setupFeaturesExtensions()
{
    // required features/extensions
    gpuFeatures11.variablePointers = VK_TRUE;
    gpuFeatures11.shaderDrawParameters = VK_TRUE;
    gpuFeatures12.samplerMirrorClampToEdge = VK_TRUE;
    gpuFeatures12.drawIndirectCount = VK_TRUE;
    gpuFeatures12.storageBuffer8BitAccess;
    gpuFeatures12.uniformAndStorageBuffer8BitAccess;
    gpuFeatures12.storagePushConstant8;
    gpuFeatures12.shaderBufferInt64Atomics = VK_TRUE;
    gpuFeatures12.shaderSharedInt64Atomics = VK_TRUE;
    gpuFeatures12.shaderFloat16;
    gpuFeatures12.shaderInt8;
    gpuFeatures12.descriptorIndexing = VK_TRUE;
    gpuFeatures12.shaderInputAttachmentArrayDynamicIndexing = VK_TRUE;
    gpuFeatures12.shaderUniformTexelBufferArrayDynamicIndexing = VK_TRUE;
    gpuFeatures12.shaderStorageTexelBufferArrayDynamicIndexing = VK_TRUE;
    gpuFeatures12.shaderUniformBufferArrayNonUniformIndexing = VK_TRUE;
    gpuFeatures12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    gpuFeatures12.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
    gpuFeatures12.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;
    gpuFeatures12.shaderInputAttachmentArrayNonUniformIndexing = VK_TRUE;
    gpuFeatures12.shaderUniformTexelBufferArrayNonUniformIndexing = VK_TRUE;
    gpuFeatures12.shaderStorageTexelBufferArrayNonUniformIndexing = VK_TRUE;
    gpuFeatures12.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
    gpuFeatures12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    gpuFeatures12.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
    gpuFeatures12.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
    gpuFeatures12.descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE;
    gpuFeatures12.descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE;
    gpuFeatures12.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
    gpuFeatures12.descriptorBindingPartiallyBound = VK_TRUE;
    gpuFeatures12.descriptorBindingVariableDescriptorCount = VK_TRUE;
    gpuFeatures12.runtimeDescriptorArray = VK_TRUE;
    gpuFeatures12.samplerFilterMinmax = VK_TRUE;
    gpuFeatures12.scalarBlockLayout = VK_TRUE;
    gpuFeatures12.uniformBufferStandardLayout = VK_TRUE;
    gpuFeatures12.timelineSemaphore = VK_TRUE;
    gpuFeatures12.bufferDeviceAddress = VK_TRUE;
    gpuFeatures12.vulkanMemoryModel = VK_TRUE;
    gpuFeatures12.vulkanMemoryModelDeviceScope = VK_TRUE;
    gpuFeatures12.vulkanMemoryModelAvailabilityVisibilityChains = VK_TRUE;

    gpuFeatures13.inlineUniformBlock = VK_TRUE;
    gpuFeatures13.descriptorBindingInlineUniformBlockUpdateAfterBind = VK_TRUE;
    gpuFeatures13.shaderDemoteToHelperInvocation = VK_TRUE;
    gpuFeatures13.shaderTerminateInvocation = VK_TRUE;
    gpuFeatures13.synchronization2 = VK_TRUE;
    gpuFeatures13.dynamicRendering = VK_TRUE;
    gpuFeatures13.shaderIntegerDotProduct = VK_TRUE;
    gpuFeatures13.maintenance4 = VK_TRUE;

    descriptorBufferFeature.descriptorBuffer = VK_TRUE;
    descriptorBufferFeature.descriptorBufferCaptureReplay = VK_TRUE;
    descriptorBufferFeature.descriptorBufferImageLayoutIgnored = VK_TRUE;
    descriptorBufferFeature.descriptorBufferPushDescriptors = VK_TRUE;

    dynamicRenderingUnusedAttachmentsFeature.dynamicRenderingUnusedAttachments = VK_TRUE;

    requestedExtensionsNames.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    requestedExtensionsNames.push_back(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    requestedExtensionsNames.push_back(VK_EXT_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_EXTENSION_NAME);

    next = &dynamicRenderingUnusedAttachmentsFeature;

    // recommended features/extensions
    if (exists<const char*>(availableExtensionsNames, VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME))
    {
        enabledRecommendedFeatures.deviceGeneratedCommands = true;
        deviceGeneratedCommandsFeature.deviceGeneratedCommands = VK_TRUE;
        deviceGeneratedCommandsFeature.pNext = next;
        next = &deviceGeneratedCommandsFeature;
        requestedExtensionsNames.push_back(VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME);
    }

    if (exists<const char*>(availableExtensionsNames, VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME))
    {
        enabledRecommendedFeatures.graphicsPipelineLibrary = true;
        graphicsPipelineLibraryFeature.graphicsPipelineLibrary = VK_TRUE;
        graphicsPipelineLibraryFeature.pNext = next;
        next = &graphicsPipelineLibraryFeature;
        requestedExtensionsNames.push_back(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    }

    if (exists<const char*>(availableExtensionsNames, VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME))
    {
        enabledRecommendedFeatures.dynamicRenderingLocalRead = true;
        dynamicRenderingLocalReadFeature.dynamicRenderingLocalRead = VK_TRUE;
        dynamicRenderingLocalReadFeature.pNext = next;
        next = &dynamicRenderingLocalReadFeature;
        requestedExtensionsNames.push_back(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME);
    }

    if (exists<const char*>(availableExtensionsNames, VK_KHR_MAINTENANCE_5_EXTENSION_NAME))
    {
        enabledRecommendedFeatures.maintenance5 = true;
        maintenance5Feature.maintenance5 = VK_TRUE;
        maintenance5Feature.pNext = next;
        next = &maintenance5Feature;
        requestedExtensionsNames.push_back(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    }

    next = &maintenance5Feature;

    // optional features/extensions

    if (exists<const char*>(availableExtensionsNames, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME))
    {
        fragmentShadingRateFeature.pipelineFragmentShadingRate = VK_TRUE;
        fragmentShadingRateFeature.pNext = next;
        next = &fragmentShadingRateFeature;
        requestedExtensionsNames.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    }

    if (exists<const char*>(availableExtensionsNames, VK_EXT_MESH_SHADER_EXTENSION_NAME))
    {
        meshShaderFeature.taskShader = VK_TRUE;
        meshShaderFeature.meshShader = VK_TRUE;
        meshShaderFeature.pNext = next;
        next = &meshShaderFeature;
        requestedExtensionsNames.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    }

}

void Device::setupDevice()
{
    // get queue families
    uint32_t queueFamiliesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamiliesCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamiliesVec(queueFamiliesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamiliesCount, queueFamiliesVec.data());

    VkQueueFlags graphicsFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT;
    VkQueueFlags transferFlags = VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT;
    VkQueueFlags computeFlags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT;

    std::vector<VkDeviceQueueCreateInfo> queueCIs;

    // find queue family indices
    uint32_t familyIdx = 0;
    float priority = 1.0f; // stack value, used one time before destruction
    for (const VkQueueFamilyProperties& queueFamily : queueFamiliesVec)
    {
        if (queueFamily.queueFlags == graphicsFlags)
        {
            VkDeviceQueueCreateInfo ci = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = familyIdx,
                .queueCount = 1,
                .pQueuePriorities = &priority,
            };
            queueCIs.push_back(ci);
        }
        familyIdx++;
    }

    VkDeviceCreateInfo deviceCI = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = next,
        .queueCreateInfoCount = (uint32_t)queueCIs.size(),
        .pQueueCreateInfos = queueCIs.data(),

    };
    handleResult(vkCreateDevice(m_physicalDevice, &deviceCI, nullptr, &m_device));
    volkLoadDevice(m_device);
}

void Device::getQueues()
{

}