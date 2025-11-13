/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_DEVICE_HPP
#define LETTUCE_CORE_DEVICE_HPP

// standard headers
#include <concepts>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

// project headers
#include "Allocators/GPUMonotonicBufferResource.hpp"
#include "Allocators/LinearImageAlloc.hpp"
#include "Allocators/LinearBufferSubAlloc.hpp"
#include "common.hpp"
#include "DescriptorTable.hpp"
#include "DeviceExecutionContext.hpp"
#include "DeviceVector.hpp"
#include "Pipeline.hpp"
#include "RenderFlowGraph.hpp"
#include "Swapchain.hpp"
#include "Sampler.hpp"
#include "SequentialExecutionContext.hpp"
#include "ShaderPack.hpp"
#include "TextureDictionary.hpp"

namespace Lettuce::Core
{
    struct GraphicsPipelineCreateData
    {
        std::weak_ptr<ShaderPack> shaders;
        std::weak_ptr<DescriptorTable> descriptorTable;
        std::vector<std::weak_ptr<RenderTarget>> colorTargets;
        std::weak_ptr<RenderTarget> depthTarget;
        // "classic" rasterization pipeline
        std::vector<VertexInput> inputs;
        std::optional<std::string> vertexEntryPoint;
        std::optional<std::string> tesselletionControlEntryPoint;
        std::optional<std::string> tesselletionEvaluationEntryPoint;
        std::optional<std::string> geometryEntryPoint;
        // mesh rasterization pipeline
        std::optional<std::string> taskEntryPoint;
        std::optional<std::string> meshEntryPoint;
        std::string fragmentEntryPoint;
    };

    // for the future
    struct RayTracingPipelineCreateData
    {
        std::weak_ptr<ShaderPack> shaders;
        std::weak_ptr<DescriptorTable> descriptorTable;
        std::optional<std::string> rayGenEntryPoint;
        std::optional<std::string> anyHitEntryPoint;
        std::optional<std::string> closestHitEntryPoint;
        std::optional<std::string> missEntryPoint;
        std::optional<std::string> intersectionEntryPoint;
        std::optional<std::string> callableEntryPoint;
    };

    struct ComputePipelineCreateData
    {
        std::weak_ptr<ShaderPack> shaders;
        std::weak_ptr<DescriptorTable> descriptorTable;
        std::string computeEntryPoint;
    };

    struct TextureCreateData
    {
        std::vector<std::pair<std::string, std::string>> namePathPairs;
    };

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

    class Device : public IDevice
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

        auto CreateContext(const DeviceExecutionContextCreateInfo& createInfo) -> Result<DeviceExecutionContext>;
        auto CreateDescriptorTable(const DescriptorTableCreateInfo& createInfo) -> Result<DescriptorTable>;

        template<typename T>
        auto CreateDeviceVector(const DeviceVectorCreateInfo& createInfo) -> Result<DeviceVector<T>>;

        template<ICommandRecordingContext... Contexts>
        auto CreateGraph() -> Result<RenderFlowGraph<Contexts...>>;
        auto CreatePipeline(const ComputePipelineCreateData& data) -> Result<Pipeline>;
        auto CreatePipeline(const GraphicsPipelineCreateData& data) -> Result<Pipeline>;
        auto CreateRenderTarget(const RenderTargetCreateInfo& createInfo) -> Result<RenderTarget>;
        // auto CreateSampler() -> Result<Sampler>;
        auto CreateSequentialContext() -> Result<SequentialExecutionContext>;
        auto CreateShaderPack(const ShaderPackCreateInfo& createInfo) -> Result<ShaderPack>;
        auto CreateSwapchain(const SwapchainCreateInfo& createInfo) -> Result<Swapchain>;
        // auto CreateTableGroup() -> Result<TableGroup>;
        auto CreateTextureDictionary(const TextureCreateData& createData) -> Result<TextureDictionary>;

        // allocators:
        auto CreateGPUMonotonicBufferResource(const Allocators::GPUMonotonicBufferResourceCreateInfo& createInfo) -> Result<Allocators::GPUMonotonicBufferResource>;
        auto CreateLinearImageAllocator(const Allocators::LinearImageAllocCreateInfo& createInfo) -> Result<Allocators::LinearImageAlloc>;
        auto CreateLinearBufferSuballocator(const Allocators::LinearBufferSubAllocCreateInfo& createInfo) -> Result<Allocators::LinearBufferSubAlloc>;
    };
}

#include "Device.inl"
#endif // LETTUCE_CORE_DEVICE_HPP