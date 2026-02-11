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
        bool RayTracing;
        bool RayTracingNV;
        bool NeuralShading;
        bool NeuralShadingNV;
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

        VkPhysicalDeviceCooperativeMatrixFeaturesKHR cooperativeMatrixFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR,
        };
        VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR computeShaderDerivativesFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_PROPERTIES_KHR,
        };

        // optional features/extensions
        VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragmentShadingRateFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR,
        };
        VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
        };

        // raytracing extensions
        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
        };
        VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeature = {
             .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR
        };
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeature = {
             .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
        };
        VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR rayTracingMaintenance1Feature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR,
        };
        VkPhysicalDeviceOpacityMicromapFeaturesEXT opacityMicromapFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_FEATURES_EXT,
        };
        VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR rayTracingPositionFetch = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR,
        };

        // raytracing NV extensions
        VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV rayTracingInvocationReorderFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_FEATURES_NV,
        };
        VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV rayTracingLinearSweptSpheresFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_LINEAR_SWEPT_SPHERES_FEATURES_NV,
        };
        VkPhysicalDeviceClusterAccelerationStructureFeaturesNV clusterAccelerationStructureFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_ACCELERATION_STRUCTURE_FEATURES_NV,
        };
        VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV partitionedAccelerationStructureFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PARTITIONED_ACCELERATION_STRUCTURE_FEATURES_NV,
        };

        // other NV extensions
        VkPhysicalDeviceCooperativeVectorFeaturesNV cooperativeVectorFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_VECTOR_FEATURES_NV,
        };
        VkPhysicalDeviceCooperativeMatrix2FeaturesNV cooperativeMatrix2Feature = {
             .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_2_FEATURES_NV,
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