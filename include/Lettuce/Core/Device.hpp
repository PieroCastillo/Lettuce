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
#include "GPU.hpp"
#include "Memory.hpp"
#include "Buffer.hpp"
#include "DescriptorTable.hpp"
#include "TextureArray.hpp"

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

    struct DescriptorTableBufferUpdateData
    {
        std::shared_ptr<Buffer> buffer;
        DescriptorAddressType addressType;
    };

    struct DescriptorTableTextureViewUpdateData
    {
        std::shared_ptr<TextureView> textureView;
        std::shared_ptr<Sampler> sampler;
        DescriptorTextureType textureType;
    };

    struct DescriptorTableSamplerUpdateData
    {
        std::shared_ptr<Sampler> sampler;
    };

    struct DescriptorTableUpdateBindingInfo
    {
        uint32_t set;
        uint32_t binding;
        std::variant<DescriptorTableBufferUpdateData, DescriptorTableTextureViewUpdateData, DescriptorTableSamplerUpdateData> updateData;
    };

    enum class DeviceQueueType
    {
        Graphics,
        Compute,
        Transfer,
    }

    struct QueueFamilyIndices
    {
        uint32_t graphics;
        uint32_t compute;
        uint32_t transfer;
    };

    class Device : IDevice
    {
    private:
        std::vector<std::string> availableExtensionsNames;
        std::vector<char*> availableLayersNames;
        std::vector<const char*> requestedExtensionsNames;
        std::vector<const char*> requestedLayersNames;
        void* next;
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

        inline void setupInstance();
        inline void setupFeaturesExtensions();
        inline void setupDevice();
        inline void getQueues();

    public:

        Device();
        void Build();
        void Release();

        EnabledRecommendedFeatures GetEnabledRecommendedFeatures()
        {
            return enabledRecommendedFeatures;
        }

        template <typename T>
        using Result = std::expected<std::shared_ptr<T>, LettuceResult>;
        using Op = std::expected<void, LettuceResult>;

        template <typename T>
        concept HasRelease = requires(T obj) {
            { obj.Release() } -> std::same_as<void>;
        };

        template <typename T, typename TCreateInfo>
        concept ConstructibleFromDevice = requires(T obj,const std::weak_ptr<IDevice>& device, TCreateInfo createInfo) {
            { obj.Create(device, createInfo) } -> std::same_as<LettuceResult>;
        };

        template <typename T, typename TCreateInfo>
            requires HasRelease<T> && ConstructibleFromDevice<T, TCreateInfo>
        auto CreateObject(const TCreateInfo& createInfo) -> Result<T>
        {
            auto obj = std::make_shared<T>();
            auto result = obj->Create(this, createInfo);

            if (result == LettuceResult::Success)
                return obj;

            return std::unexpected(result);
        }

        auto UpdateBinding(const std::shared_ptr<DescriptorTable> descriptorTable, const DescriptorTableUpdateBindingInfo& updateInfo) -> Op;

        auto BindMemory(std::shared_ptr<Memory> memory, std::shared_ptr<Buffer> buffer) -> Op;
        auto BindMemory(std::shared_ptr<Memory> memory, std::shared_ptr<TextureArray> textureArray) -> Op;

        auto MemoryAlloc(uint32_t size = (16 * 1024 * 1024), MemoryAccess access = MemoryAccess::FastGPUReadWrite) -> Result<Memory>; // 16 MiB default size
        auto MemoryCopy(std::shared_ptr<Buffer> srcBuffer, std::shared_ptr<Buffer> dstBuffer) -> Op;
        auto MemoryCopy(std::shared_ptr<Buffer> srcBuffer, std::shared_ptr<TextureArray> dstTextureArray) -> Op;
        auto MemoryCopy(std::shared_ptr<TextureArray> srcTextureArray, std::shared_ptr<Buffer> dstBuffer) -> Op;
        auto MemoryCopy(std::shared_ptr<TextureArray> srcTextureArray, std::shared_ptr<TextureArray> dstTextureArray) -> Op;

        auto Blit(std::shared_ptr<TextureArray> srcTextureArray, std::shared_ptr<TextureArray> dstTextureArray) -> Op;

        auto Present(const std::shared_ptr<Swapchain>& swapchain) -> Op;
    };
}
#endif // LETTUCE_CORE_DEVICE_HPP