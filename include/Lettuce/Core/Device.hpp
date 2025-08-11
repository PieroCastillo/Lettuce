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
#include "Buffer.hpp"
#include "CommandList.hpp"
#include "ComputeNode.hpp"
#include "DescriptorTable.hpp"
#include "GPU.hpp"
#include "Memory.hpp"
#include "Pipeline.hpp"
#include "PipelineLayout.hpp"
#include "RenderFlowGraph.hpp"
#include "RenderNode.hpp"
#include "RenderTarget.hpp"
#include "Sampler.hpp"
#include "Swapchain.hpp"
#include "TextureArray.hpp"
#include "TextureView.hpp"
#include "TransferNode.hpp"

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

    enum class LettuceResult
    {
        OutOfDeviceMemory,
        OutOfHostMemory,
        Unknown,
    };

    enum class DeviceQueueType
    {
        Graphics,
        Compute,
        Transfer,
    }

    class Device
    {
    private:
        std::vector<std::string> availableExtensionsNames;
        std::vector<char *> availableLayersNames;
        std::vector<const char *> requestedExtensionsNames;
        std::vector<const char *> requestedLayersNames;
        void *next;
        Features _features;
        Features _enabledFeatures;
        EnabledRecommendedFeatures enabledRecommendedFeatures;
        std::unordered_multimap<DeviceQueueType, VkQueue> queues;

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
        VkInstance m_instance;
        VkPhysicalDevice m_physicalDevice;
        VkDevice m_device;

        Device();
        void Build();
        void Release();

        EnabledRecommendedFeatures GetEnabledRecommendedFeatures()
        {
            return enabledRecommendedFeatures;
        }

        
        template <typename T>
        using Result = std::expected<std::shared_ptr<T>, LettuceResult>;
        template <typename T>
        using AsyncResult = std::future<Result<T>>;
        using Op = std::expected<void, LettuceResult>;
        using AsyncOp = std::future<Op>;

        template <typename T, typename TCreateInfo>
        concept ValidObjectType = std::constructible_from<T, VkDevice, TCreateInfo> && requires(T obj) {
            { obj.Release(); } -> std::same_as<void>;
        };

        template <typename T, typename Args>
        requires ValidObjectType<T, Args>
        inline auto CreateObject(Args &&args) -> Result<T>
        {
            return std::make_shared<T>(m_device, std::forward<Args>(args));
        }

        template <typename T, typename Args>
        requires ValidObjectType<T, Args>
        inline auto CreateAsyncObject(Args && args) -> AsyncResult<T>
        {
            return std::async<Result<T>>(std::launch::async, std::make_shared<T>(m_device, std::forward<Args>(args)));
        }

        auto UpdateBinding(const std::shared_ptr<DescriptorTable> descriptorTable, const DescriptorTableUpdateBindingInfo &updateInfo) -> Op;

        auto BindMemory(std::shared_ptr<Memory> memory, std::shared_ptr<Buffer> buffer) -> Op;
        auto BindMemory(std::shared_ptr<Memory> memory, std::shared_ptr<TextureArray> textureArray) -> Op;

        auto MemoryAlloc(uint32_t size = (16 * 1024 * 1024), MemoryAccess access = MemoryAccess::FastGPUReadWrite) -> Result<Memory> // 16 MiB default size
        {
            MemoryCreateInfo createInfo = {
                .size = size,
                .access = access,
            };
            return std::make_shared<Memory>(m_device, createInfo);
        }
        auto MemoryCopy(std::shared_ptr<Buffer> srcBuffer, std::shared_ptr<Buffer> dstBuffer) -> Op;
        auto MemoryCopy(std::shared_ptr<Buffer> srcBuffer, std::shared_ptr<TextureArray> dstTextureArray) -> Op;
        auto MemoryCopy(std::shared_ptr<TextureArray> srcTextureArray, std::shared_ptr<Buffer> dstBuffer) -> Op;
        auto MemoryCopy(std::shared_ptr<TextureArray> srcTextureArray, std::shared_ptr<TextureArray> dstTextureArray) -> Op;

        auto Blit(std::shared_ptr<TextureArray> srcTextureArray, std::shared_ptr<TextureArray> dstTextureArray) -> Op;
    };
}
#endif // LETTUCE_CORE_DEVICE_HPP