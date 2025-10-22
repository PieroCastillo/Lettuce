/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_DEVICE_HPP
#define LETTUCE_CORE_DEVICE_HPP

// standard headers
#include <memory>
#include <variant>
#include <expected>
#include <vector>
#include <future>
#include <unordered_map>
#include <concepts>

// project headers
#include "common.hpp"
#include "CommandList.hpp"
#include "DescriptorTable.hpp"
#include "DeviceExecutionContext.hpp"
#include "Swapchain.hpp"
#include "Sampler.hpp"

namespace Lettuce::Core
{
    struct Features
    {
        bool FragmentShadingRate;
        bool ExecutionGraphs;
        bool MeshShading;
        bool RayTracing;
        bool Video;
    };

    struct EnabledRecommendedFeatures
    {
        bool shaderObject = false;
        bool deviceGeneratedCommands = false;
        bool graphicsPipelineLibrary = false;
        bool dynamicRenderingLocalRead = false;
        bool maintenance5 = false;
    };

    enum class DeviceQueueType
    {
        Graphics,
        Compute,
        Transfer,
    };

    struct QueueFamilyIndices
    {
        uint32_t graphics;
        uint32_t compute;
        uint32_t transfer;
    };

    struct DeviceCreateInfo
    {
        bool preferDedicated;
    };

    class Device : IDevice
    {
    private:
        VkDebugUtilsMessengerEXT m_messenger;
        std::vector<const char*> availableExtensionsNames;
        std::vector<char*> availableLayersNames;
        std::vector<const char*> requestedExtensionsNames;
        std::vector<const char*> requestedLayersNames;
        void* next = nullptr;
        Features _features;
        Features _enabledFeatures;
        EnabledRecommendedFeatures enabledRecommendedFeatures;
        std::unordered_multimap<DeviceQueueType, VkQueue> queues;
        QueueFamilyIndices queuefamilyIndices;

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

        void setupInstance();
        void selectGPU(const DeviceCreateInfo& createInfo);
        void setupFeaturesExtensions();
        void setupDevice();
        void getQueues();

    public:
        void Create(const DeviceCreateInfo& createInfo);
        void Release();

        EnabledRecommendedFeatures GetEnabledRecommendedFeatures()
        {
            return enabledRecommendedFeatures;
        }

        template <typename T>
        using Result = std::expected<std::shared_ptr<T>, LettuceResult>;
        using Op = std::expected<void, LettuceResult>;

        auto CreateSwapchain(const SwapchainCreateInfo& createInfo) -> Result<Swapchain>;
        // auto CreateDescriptorTable() -> Result<DescriptorTable>;
        auto CreateContext(const DeviceExecutionContextCreateInfo& createInfo) -> Result<DeviceExecutionContext>;
        // auto CreateRenderTarget() -> Result<RenderTarget>;
        // auto CreatePipelineLayout() -> Result<PipelineLayout>;
        // auto CreatePipeline() -> Result<Pipeline>;
        // auto CreateSampler() -> Result<Sampler>;
        // auto CreateTableGroup() -> Result<TableGroup>;
        // auto CreateTextureDictionary() -> Result<TextureDictionary>;

        auto Present(const std::shared_ptr<Swapchain>& swapchain) -> Op;
    };
}
#endif // LETTUCE_CORE_DEVICE_HPP