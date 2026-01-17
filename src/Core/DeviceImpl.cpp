// standard headers
#include <vector>
#include <iostream>
#include <print>

// project headers
#include "Lettuce/Core/DeviceImpl.hpp"

using namespace Lettuce::Core;

auto exists(const std::vector<std::string>& list, const char* key) -> bool
{
    return std::find_if(list.begin(), list.end(),
        [&](const std::string& s) {
            return s == key;   // comparación por contenido
        }) != list.end();
}

#define COLOR_RESET   "\033[0m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_WHITE   "\033[0;37m"

// Debug Callback :D (smile face because I don't like this syntax :p)
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    // Warning
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        std::cout << COLOR_YELLOW << "[WARNING] " << COLOR_WHITE
            << pCallbackData->pMessage << COLOR_RESET << std::endl;
    }
    // Error
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        std::cout << COLOR_RED << "[ERROR] " << COLOR_WHITE
            << pCallbackData->pMessage << COLOR_RESET << std::endl;
    }
    // // Info or verbose messages
    // else {
    //     std::cout << COLOR_WHITE << pCallbackData->pMessage << COLOR_RESET << std::endl;
    // }

    return VK_FALSE;
}

void DeviceImpl::Create(const DeviceCreateInfo& createInfo)
{
    setupInstance();
    selectGPU(createInfo);
    setupFeaturesExtensions();
    setupDevice();
}

void DeviceImpl::Release()
{
    handleResult(vkDeviceWaitIdle(m_device));
    vkDestroySemaphore(m_device, graphicsSemaphore, nullptr);
    vkDestroySemaphore(m_device, computeSemaphore, nullptr);
    vkDestroySemaphore(m_device, transferSemaphore, nullptr);
    vkDestroyDevice(m_device, nullptr);
    vkDestroyDebugUtilsMessengerEXT(m_instance, m_messenger, nullptr);
    vkDestroyInstance(m_instance, nullptr);
    volkFinalize();
}

void DeviceImpl::setupInstance()
{
    VkResult res;
    try
    {
        res = volkInitialize();
    }
    catch (...)
    {
        std::cerr << "Volk initialization failed: " << res << std::endl;
        return;
    }

    // create info about app & engine
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Lettuce Application",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "Lettuce Rendering Library",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_3,
    };

    std::vector<const char*> instanceExtensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(WIN32_) || defined(_WIN32) || defined(WIN32)
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    };

    std::array<const char*, 1> layers = {
        "VK_LAYER_KHRONOS_validation",
    };

    VkDebugUtilsMessengerCreateInfoEXT messengerCI = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        // give messages about behaviours that may cause bugs or crashes
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ,
        // performance and validation messages are priority
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT ,
        .pfnUserCallback = &debugCallback,
    };

    VkInstanceCreateInfo instanceCI = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = &messengerCI,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = (uint32_t)layers.size(), // Add required layers here
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = (uint32_t)instanceExtensions.size(), // Add required extensions here
        .ppEnabledExtensionNames = instanceExtensions.data(),
    };
    handleResult(vkCreateInstance(&instanceCI, nullptr, &m_instance));
    volkLoadInstanceOnly(m_instance);

    handleResult(vkCreateDebugUtilsMessengerEXT(m_instance, &messengerCI, nullptr, &m_messenger));
}

void DeviceImpl::selectGPU(const DeviceCreateInfo& createInfo)
{
    uint32_t gpuCount;
    vkEnumeratePhysicalDevices(m_instance, &gpuCount, nullptr);
    std::vector<VkPhysicalDevice> gpus(gpuCount);
    vkEnumeratePhysicalDevices(m_instance, &gpuCount, gpus.data());

    // TODO: impl selection by properties
    m_physicalDevice = gpus[0];
}

void DeviceImpl::setupFeaturesExtensions()
{
    uint32_t extPropsCount;
    vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extPropsCount, nullptr);
    std::vector<VkExtensionProperties> extProps(extPropsCount);
    vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extPropsCount, extProps.data());

    for (const auto& extProp : extProps)
    {
        availableExtensionsNames.emplace_back(extProp.extensionName);
    }

    // required features/extensions
    gpuFeatures11.variablePointersStorageBuffer = VK_TRUE;
    gpuFeatures11.variablePointers = VK_TRUE;
    gpuFeatures11.shaderDrawParameters = VK_TRUE;

    gpuFeatures12.samplerMirrorClampToEdge = VK_TRUE;
    gpuFeatures12.drawIndirectCount = VK_TRUE;
    gpuFeatures12.storageBuffer8BitAccess = VK_TRUE;
    gpuFeatures12.uniformAndStorageBuffer8BitAccess = VK_TRUE;
    gpuFeatures12.storagePushConstant8 = VK_FALSE;
    gpuFeatures12.shaderBufferInt64Atomics = VK_TRUE;
    gpuFeatures12.shaderSharedInt64Atomics = VK_TRUE;
    gpuFeatures12.shaderFloat16 = VK_FALSE;
    gpuFeatures12.shaderInt8 = VK_FALSE;
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
    gpuFeatures12.vulkanMemoryModelAvailabilityVisibilityChains = VK_FALSE;
    gpuFeatures12.pNext = &gpuFeatures11;

    gpuFeatures13.inlineUniformBlock = VK_TRUE;
    gpuFeatures13.descriptorBindingInlineUniformBlockUpdateAfterBind = VK_TRUE;
    gpuFeatures13.shaderDemoteToHelperInvocation = VK_TRUE;
    gpuFeatures13.shaderTerminateInvocation = VK_TRUE;
    gpuFeatures13.synchronization2 = VK_TRUE;
    gpuFeatures13.dynamicRendering = VK_TRUE;
    gpuFeatures13.shaderIntegerDotProduct = VK_TRUE;
    gpuFeatures13.maintenance4 = VK_TRUE;
    gpuFeatures13.pNext = &gpuFeatures12;

    descriptorBufferFeature.descriptorBuffer = VK_TRUE;
    descriptorBufferFeature.descriptorBufferCaptureReplay = VK_FALSE;
    descriptorBufferFeature.descriptorBufferImageLayoutIgnored = VK_FALSE;
    descriptorBufferFeature.descriptorBufferPushDescriptors = VK_TRUE;
    descriptorBufferFeature.pNext = &gpuFeatures13;

    dynamicRenderingUnusedAttachmentsFeature.dynamicRenderingUnusedAttachments = VK_TRUE;
    dynamicRenderingUnusedAttachmentsFeature.pNext = &descriptorBufferFeature;

    fragmentShaderBarycentricsFeature.fragmentShaderBarycentric = VK_TRUE;
    fragmentShaderBarycentricsFeature.pNext = &dynamicRenderingUnusedAttachmentsFeature;

    next = &fragmentShaderBarycentricsFeature;

    requestedExtensionsNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    requestedExtensionsNames.push_back(VK_GOOGLE_HLSL_FUNCTIONALITY1_EXTENSION_NAME);
    // requestedExtensionsNames.push_back(VK_GOOGLE_USER_TYPE_EXTENSION_NAME);

    requestedExtensionsNames.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
    requestedExtensionsNames.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    requestedExtensionsNames.push_back(VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME);
    requestedExtensionsNames.push_back(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    requestedExtensionsNames.push_back(VK_EXT_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_EXTENSION_NAME);
    requestedExtensionsNames.push_back(VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME);

    // recommended features/extensions
    if (exists(availableExtensionsNames, VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME))
    {
        features.DeviceGeneratedCommands = true;

        deviceGeneratedCommandsFeature.deviceGeneratedCommands = VK_TRUE;
        deviceGeneratedCommandsFeature.pNext = next;
        next = &deviceGeneratedCommandsFeature;

        requestedExtensionsNames.push_back(VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME);
    }

    if (exists(availableExtensionsNames, VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME))
    {
        features.GraphicsPipelineLibrary = true;

        graphicsPipelineLibraryFeature.graphicsPipelineLibrary = VK_TRUE;
        graphicsPipelineLibraryFeature.pNext = next;
        next = &graphicsPipelineLibraryFeature;

        requestedExtensionsNames.push_back(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    }

    if (exists(availableExtensionsNames, VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME))
    {
        features.DynamicRenderingLocalRead = true;

        dynamicRenderingLocalReadFeature.dynamicRenderingLocalRead = VK_TRUE;
        dynamicRenderingLocalReadFeature.pNext = next;
        next = &dynamicRenderingLocalReadFeature;

        requestedExtensionsNames.push_back(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME);
    }

    if (exists(availableExtensionsNames, VK_KHR_MAINTENANCE_5_EXTENSION_NAME))
    {
        features.Maintenance5 = true;

        maintenance5Feature.maintenance5 = VK_TRUE;
        maintenance5Feature.pNext = next;
        next = &maintenance5Feature;

        requestedExtensionsNames.push_back(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    }

    // optional features/extensions

    if (exists(availableExtensionsNames, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME))
    {
        features.FragmentShadingRate = true;

        fragmentShadingRateFeature.pipelineFragmentShadingRate = VK_TRUE;
        fragmentShadingRateFeature.pNext = next;
        next = &fragmentShadingRateFeature;
        requestedExtensionsNames.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    }

    if (exists(availableExtensionsNames, VK_EXT_MESH_SHADER_EXTENSION_NAME))
    {
        features.MeshShading = true;

        meshShaderFeature.taskShader = VK_TRUE;
        meshShaderFeature.meshShader = VK_TRUE;
        meshShaderFeature.pNext = next;
        next = &meshShaderFeature;
        requestedExtensionsNames.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    }
}

void DeviceImpl::setupDevice()
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
    float priority = 1.0f; // stack value, used one time before destruction
    VkDeviceQueueCreateInfo queueCI = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueCount = 1,
        .pQueuePriorities = &priority,
    };

    uint32_t graphicsQueueFamilyIndex;
    uint32_t computeQueueFamilyIndex;
    uint32_t transferQueueFamilyIndex;
    uint32_t familyIdx = 0;
    for (const auto& queueFamily : queueFamiliesVec)
    {
        if (queueFamily.queueFlags == graphicsFlags)
        {
            graphicsQueueFamilyIndex = familyIdx;
        }
        else if (queueFamily.queueFlags == computeFlags)
        {
            computeQueueFamilyIndex = familyIdx;
        }
        else if (queueFamily.queueFlags == transferFlags)
        {
            transferQueueFamilyIndex = familyIdx;
        }
        familyIdx++;
    }
    queueCI.queueFamilyIndex = graphicsQueueFamilyIndex;
    queueCIs.push_back(queueCI);
    queueCI.queueFamilyIndex = computeQueueFamilyIndex;
    queueCIs.push_back(queueCI);
    queueCI.queueFamilyIndex = transferQueueFamilyIndex;
    queueCIs.push_back(queueCI);

    VkPhysicalDeviceFeatures features = {
        .multiDrawIndirect = VK_TRUE,
        .samplerAnisotropy = VK_TRUE,
        .textureCompressionBC = VK_TRUE,
        .shaderFloat64 = VK_TRUE,
        .shaderInt64 = VK_TRUE,
        .shaderInt16 = VK_TRUE,
    };

    VkPhysicalDeviceFeatures2 deviceFeatures2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = next,
        .features = features,
    };

    VkDeviceCreateInfo deviceCI = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &deviceFeatures2,
        .queueCreateInfoCount = (uint32_t)queueCIs.size(),
        .pQueueCreateInfos = queueCIs.data(),
        .enabledExtensionCount = (uint32_t)requestedExtensionsNames.size(),
        .ppEnabledExtensionNames = requestedExtensionsNames.data(),
    };
    handleResult(vkCreateDevice(m_physicalDevice, &deviceCI, nullptr, &m_device));
    volkLoadDevice(m_device);

    // get queues
    vkGetDeviceQueue(m_device, graphicsQueueFamilyIndex, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, computeQueueFamilyIndex, 0, &m_computeQueue);
    vkGetDeviceQueue(m_device, transferQueueFamilyIndex, 0, &m_transferQueue);

    VkPhysicalDeviceDescriptorBufferPropertiesEXT dbProps =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT,
    };

    VkPhysicalDeviceProperties2 props2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        .pNext = &dbProps,
    };
    vkGetPhysicalDeviceProperties2(m_physicalDevice, &props2);

    props.sampledImageDescriptorSize = dbProps.sampledImageDescriptorSize;
    props.samplerDescriptorSize = dbProps.samplerDescriptorSize;
    props.storageImageDescriptorSize = dbProps.storageImageDescriptorSize;
    props.graphicsQueueFamilyIdx = graphicsQueueFamilyIndex;
    props.computeQueueFamilyIdx = computeQueueFamilyIndex;
    props.transferQueueFamilyIdx = transferQueueFamilyIndex;
    props.maxSamplerAnisotropy = props2.properties.limits.maxSamplerAnisotropy;
    props.maxPushAllocationsCount = props2.properties.limits.maxPushConstantsSize / sizeof(uint64_t);

    graphicsCurrentValue = 0;
    computeCurrentValue = 0;
    transferCurrentValue = 0;

    VkSemaphoreTypeCreateInfo semTypeCI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = 0,
    };
    VkSemaphoreCreateInfo semCI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &semTypeCI,
    };
    handleResult(vkCreateSemaphore(m_device, &semCI, nullptr, &graphicsSemaphore));
    handleResult(vkCreateSemaphore(m_device, &semCI, nullptr, &computeSemaphore));
    handleResult(vkCreateSemaphore(m_device, &semCI, nullptr, &transferSemaphore));
}

void DeviceImpl::setDebugName(VkObjectType type, uint64_t handle, const std::string& name)
{
    if (isDebug())
    {
        VkDebugUtilsObjectNameInfoEXT nameInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .objectType = type,
            .objectHandle = handle,
            .pObjectName = name.c_str(),
        };
        handleResult(vkSetDebugUtilsObjectNameEXT(m_device, &nameInfo));
    }
}