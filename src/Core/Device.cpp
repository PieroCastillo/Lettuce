// standard headers
#include <memory>
#include <variant>
#include <expected>
#include <vector>
#include <future>
#include <unordered_map>
#include <array>
#include <iostream>

// project headers
#include "Lettuce/Core/Device.hpp"

// external headers
#include <ktx.h>
#define VK_NO_PROTOTYPE
#include <ktxvulkan.h>

using namespace Lettuce::Core;

template <typename T>
inline bool exists(const std::vector<T>& vec, const T& elem)
{
    return std::find(vec.begin(), vec.end(), elem) != vec.end();
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
        std::cerr << COLOR_YELLOW << "[WARNING] " << COLOR_WHITE
            << pCallbackData->pMessage << COLOR_RESET << std::endl;
    }
    // Error
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        std::cerr << COLOR_RED << "[ERROR] " << COLOR_WHITE
            << pCallbackData->pMessage << COLOR_RESET << std::endl;
    }
    // // Info or verbose messages
    // else {
    //     std::cout << COLOR_WHITE << pCallbackData->pMessage << COLOR_RESET << std::endl;
    // }

    return VK_FALSE;
}

void Device::Create(const DeviceCreateInfo& createInfo)
{
    setupInstance();
    selectGPU(createInfo);
    setupFeaturesExtensions();
    setupDevice();
}

void Device::Release()
{
    vkDestroyDevice(m_device, nullptr);
    vkDestroyDebugUtilsMessengerEXT(m_instance, m_messenger, nullptr);
    vkDestroyInstance(m_instance, nullptr);
    volkFinalize();
}

void Device::setupInstance()
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


void Device::selectGPU(const DeviceCreateInfo& createInfo)
{
    uint32_t gpuCount;
    vkEnumeratePhysicalDevices(m_instance, &gpuCount, nullptr);
    std::vector<VkPhysicalDevice> gpus(gpuCount);
    vkEnumeratePhysicalDevices(m_instance, &gpuCount, gpus.data());

    // TODO: impl selection by properties
    m_physicalDevice = gpus[0];
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

    requestedExtensionsNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

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
    float priority = 1.0f; // stack value, used one time before destruction
    VkDeviceQueueCreateInfo queueCI = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueCount = 1,
        .pQueuePriorities = &priority,
    };

    uint32_t familyIdx = 0;
    for (const auto& queueFamily : queueFamiliesVec)
    {
        if (queueFamily.queueFlags == graphicsFlags)
        {
            m_graphicsQueueFamilyIndex = familyIdx;
        }
        else if (queueFamily.queueFlags == computeFlags)
        {
            m_computeQueueFamilyIndex = familyIdx;
        }
        else if (queueFamily.queueFlags == transferFlags)
        {
            m_transferQueueFamilyIndex = familyIdx;
        }
        familyIdx++;
    }
    queueCI.queueFamilyIndex = m_graphicsQueueFamilyIndex;
    queueCIs.push_back(queueCI);
    queueCI.queueFamilyIndex = m_computeQueueFamilyIndex;
    queueCIs.push_back(queueCI);
    queueCI.queueFamilyIndex = m_transferQueueFamilyIndex;
    queueCIs.push_back(queueCI);

    VkDeviceCreateInfo deviceCI = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = next,
        .queueCreateInfoCount = (uint32_t)queueCIs.size(),
        .pQueueCreateInfos = queueCIs.data(),
        .enabledExtensionCount = (uint32_t)requestedExtensionsNames.size(),
        .ppEnabledExtensionNames = requestedExtensionsNames.data(),
    };
    handleResult(vkCreateDevice(m_physicalDevice, &deviceCI, nullptr, &m_device));
    volkLoadDevice(m_device);

    // get queues
    vkGetDeviceQueue(m_device, m_graphicsQueueFamilyIndex, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_computeQueueFamilyIndex, 0, &m_computeQueue);
    vkGetDeviceQueue(m_device, m_transferQueueFamilyIndex, 0, &m_transferQueue);
}

auto Device::CreateSwapchain(const SwapchainCreateInfo& createInfo) -> Result<Swapchain>
{
    try
    {
        auto swapchain = std::make_shared<Swapchain>();
        swapchain->Create(*this, createInfo);
        return swapchain;
    }
    catch (...)
    {
        return std::unexpected(LettuceResult::InitializationFailed);
    }
}

// here CreateDescriptorTable()

auto Device::CreateContext(const DeviceExecutionContextCreateInfo& createInfo) -> Result<DeviceExecutionContext>
{
    try
    {
        auto ctx = std::make_shared<DeviceExecutionContext>();
        ctx->Create(*this, createInfo);
        return ctx;
    }
    catch (...)
    {
        return std::unexpected(LettuceResult::InitializationFailed);
    }
}

auto Device::CreateGraph() -> Result<RenderFlowGraph>
{
    try
    {
        auto graph = std::make_shared<RenderFlowGraph>();
        RenderFlowGraphCreateInfo ci = {

        };
        graph->Create(*this, ci);
        return graph;
    }
    catch (...)
    {
        return std::unexpected(LettuceResult::InitializationFailed);
    }
}

auto Device::CreateTextureDictionary(const TextureCreateData& createData) -> Result<TextureDictionary>
{
    try
    {
        auto dict = std::make_shared<TextureDictionary>();
        // retrieve file names and info
        TextureDictionaryCreateInfo textureCI;
        std::vector<ktxTexture*> ktxTextures;

        for(const auto& [name, path] : createData.namePathPairs)
        {
            // get params
            ktxTexture* texture;
            auto res = ktxTexture_CreateFromNamedFile(path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texture);
            
            textureCI.names.push_back(std::move(name));
            textureCI.formats.push_back(ktxTexture_GetVkFormat(texture));
            textureCI.widths.push_back(texture->baseWidth);
            textureCI.heights.push_back(texture->baseHeight);
            textureCI.layerCounts.push_back(texture->numLayers);
            textureCI.levelCounts.push_back(texture->numLevels);
            textureCI.isCubes.push_back(texture->isCubemap);

            ktxTextures.push_back(texture);
        }

        dict->Create(*this, textureCI);
        // TODO: transfer data

        // destroy ktx data
        for(const auto& tex : ktxTextures)
        {
            ktxTexture_Destroy(tex);
        }

        return dict;
    }
    catch(...)
    {
        throw LettuceException(LettuceResult::InitializationFailed);
    }
}