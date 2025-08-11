// standard headers
#include <memory>
#include <variant>
#include <expected>
#include <vector>
#include <future>
#include <unordered_map>
#include <array>

// project headers
#include "Lettuce/Core/Buffer.hpp"
#include "Lettuce/Core/CommandList.hpp"
#include "Lettuce/Core/ComputeNode.hpp"
#include "Lettuce/Core/DescriptorTable.hpp"
#include "Lettuce/Core/GPU.hpp"
#include "Lettuce/Core/Memory.hpp"
#include "Lettuce/Core/Pipeline.hpp"
#include "Lettuce/Core/PipelineLayout.hpp"
#include "Lettuce/Core/RenderFlowGraph.hpp"
#include "Lettuce/Core/RenderNode.hpp"
#include "Lettuce/Core/RenderTarget.hpp"
#include "Lettuce/Core/Sampler.hpp"
#include "Lettuce/Core/Swapchain.hpp"
#include "Lettuce/Core/TextureArray.hpp"
#include "Lettuce/Core/TextureView.hpp"
#include "Lettuce/Core/TransferNode.hpp"

using namespace Lettuce::Core;

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
}

void Device::setupInstance()
{
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Lettuce Application",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "Lettuce Rendering Library",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_3,
    };

    std::array<const char *, 2> instanceExtensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
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
    vkCreateInstance(&instanceCI, nullptr, &m_instance);
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

    // recommended features/extensions
    deviceGeneratedCommandsFeature.deviceGeneratedCommands = VK_TRUE;
    graphicsPipelineLibraryFeature.graphicsPipelineLibrary = VK_TRUE;

    dynamicRenderingLocalReadFeature.dynamicRenderingLocalRead = VK_TRUE;
    maintenance5Feature.maintenance5 = VK_TRUE;

    // optional features/extensions
    fragmentShadingRateFeature.fragmentShadingRate = VK_TRUE;
    meshShaderFeature.taskShader = VK_TRUE;
    meshShaderFeature.meshShader = VK_TRUE;

}

void Device::setupDevice()
{
    VkDeviceCreateInfo deviceCI = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    };
    vkCreateDevice(m_physicalDevice, &deviceCI, nullptr, &m_device);
}

void Device::getQueues()
{
}

auto UpdateBinding(const std::shared_ptr<DescriptorTable> descriptorTable, const DescriptorTableUpdateBindingInfo &updateInfo) -> Op
{
}

auto BindMemory(std::shared_ptr<Memory> memory, std::shared_ptr<Buffer> buffer) -> Op
{
}

auto BindMemory(std::shared_ptr<Memory> memory, std::shared_ptr<TextureArray> textureArray) -> Op
{
}

auto MemoryAlloc(uint32_t size) -> Result<Memory>
{
}

auto MemoryCopy(std::shared_ptr<Buffer> srcBuffer, std::shared_ptr<Buffer> dstBuffer) -> Op
{
}

auto MemoryCopy(std::shared_ptr<Buffer> srcBuffer, std::shared_ptr<TextureArray> dstTextureArray) -> Op
{
}

auto MemoryCopy(std::shared_ptr<TextureArray> srcTextureArray, std::shared_ptr<Buffer> dstBuffer) -> Op
{
}
auto MemoryCopy(std::shared_ptr<TextureArray> srcTextureArray, std::shared_ptr<TextureArray> dstTextureArray) -> Op
{
}

auto Blit(std::shared_ptr<TextureArray> srcTextureArray, std::shared_ptr<TextureArray> dstTextureArray) -> Op
{
}