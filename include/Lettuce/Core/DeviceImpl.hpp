/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_DEVICE_IMPL_HPP
#define LETTUCE_CORE_DEVICE_IMPL_HPP

// standard headers
#include <concepts>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

// project headers
#include "api.hpp"
#include "common.hpp"
#include "HelperStructs.hpp"
#include "ResourcePool.hpp"

namespace Lettuce::Core
{
    struct Features
    {
        bool FragmentShadingRate;
        bool MeshShading;
        bool DeviceGeneratedCommands;
        bool GraphicsPipelineLibrary;
        bool DynamicRenderingLocalRead;
        bool Maintenance5;
    };

    struct Properties
    {
        uint32_t sampledImageDescriptorSize;
        uint32_t samplerDescriptorSize;
        uint32_t storageImageDescriptorSize;
        uint32_t graphicsQueueFamilyIdx;
        uint32_t computeQueueFamilyIdx;
        uint32_t transferQueueFamilyIdx;
        uint32_t maxPushAllocationsCount;
        float maxSamplerAnisotropy;
    };

    enum class DeviceQueueType
    {
        Graphics,
        Compute,
        Transfer,
    };

    struct DeviceCreateInfo
    {
        bool preferDedicated;
    };

    class DeviceImpl
    {
    public:
        VkDevice m_device;
        VkInstance m_instance;
        VkPhysicalDevice m_physicalDevice;
        VkQueue m_graphicsQueue;
        VkQueue m_computeQueue;
        VkQueue m_transferQueue;

        VkSemaphore graphicsSemaphore, computeSemaphore, transferSemaphore;
        uint64_t graphicsCurrentValue, computeCurrentValue, transferCurrentValue;

        bool supportBufferUsage2;
        VkDebugUtilsMessengerEXT m_messenger;
        std::vector<std::string> availableExtensionsNames;
        std::vector<char*> availableLayersNames;
        std::vector<const char*> requestedExtensionsNames;
        std::vector<const char*> requestedLayersNames;
        void* next = nullptr;
        std::unordered_multimap<DeviceQueueType, VkQueue> queues;

        Features features;
        Properties props;

        ResourcePool<MemoryHeap, MemoryHeapVK> memoryHeaps;
        ResourcePool<Buffer, BufferVK> buffers;
        ResourcePool<Texture, TextureVK> textures;
        ResourcePool<RenderTarget, RenderTargetVK> renderTargets;
        ResourcePool<Sampler, VkSampler> samplers;
        ResourcePool<ShaderBinary, VkShaderModule> shaders;
        ResourcePool<Pipeline, PipelineVK> pipelines;
        ResourcePool<DescriptorTable, DescriptorTableVK> descriptorTables;
        ResourcePool<IndirectSet, IndirectSetVK> indirectSets;
        ResourcePool<Swapchain, SwapchainVK> swapchains;
        ResourcePool<CommandAllocator, CommandAllocatorVK> commandAllocators;

        // physical device features structs
        // required features/extensions
        VkPhysicalDeviceVulkan11Features gpuFeatures11 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        };
        VkPhysicalDeviceVulkan12Features gpuFeatures12 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &gpuFeatures11,
        };
        VkPhysicalDeviceVulkan13Features gpuFeatures13 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = &gpuFeatures12,
        };
        VkPhysicalDeviceDescriptorBufferFeaturesEXT descriptorBufferFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT,
            .pNext = &gpuFeatures13,
        };
        VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT dynamicRenderingUnusedAttachmentsFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT,
            .pNext = &descriptorBufferFeature,
        };
        VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR fragmentShaderBarycentricsFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR,
        };
        // recommended features/extensions
        VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT deviceGeneratedCommandsFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_EXT,
        };

        VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT graphicsPipelineLibraryFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT,
        };

        VkPhysicalDeviceDynamicRenderingLocalReadFeaturesKHR dynamicRenderingLocalReadFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES_KHR,
        };

        VkPhysicalDeviceMaintenance5FeaturesKHR maintenance5Feature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR,
        };

        // optional features/extensions
        VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragmentShadingRateFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR,
        };

        VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
        };

        void setDebugName(VkObjectType type, uint64_t handle, const std::string& name);

        void setupInstance();
        void selectGPU(const DeviceCreateInfo& createInfo);
        void setupFeaturesExtensions();
        void setupDevice();

        void Create(const DeviceCreateInfo& createInfo);
        void Release();

        bool isDebug() { return true; };
    };
}
#endif // LETTUCE_CORE_DEVICE_IMPL_HPP