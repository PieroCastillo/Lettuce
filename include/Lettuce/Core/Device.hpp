//
// Created by piero on 14/02/2024.
//
#pragma once

#include <iostream>
#include <set>
#include <vector>
#include <string>
#include "IReleasable.hpp"
#include "Instance.hpp"
#include "GPU.hpp"

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
        bool graphicPipelineLibrary = false;
        bool dynamicRenderingLocalRead = false;
    };

    class Device : public IReleasable
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
        VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
        };

        VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT deviceGeneratedCommandsFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_EXT,
        };

        VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT graphicsPipelineLibraryFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT,
        };

        VkPhysicalDeviceDynamicRenderingLocalReadFeaturesKHR dynamicRenderingLocalReadFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES_KHR,
        };

        // optional features/extensions
        VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragmentShadingRateFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR,
        };

        VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
        };

        void addExt(const std::string &extName);

        bool checkExtIfExists(const std::string &extName);

        void addRequiredFeatures();

        void addRecommendedFeatures();

        void addOptionalFeatures();

        void listExtensions();

        void listLayers();

        void loadExtensionsLayersAndFeatures();

    public:
        std::shared_ptr<Instance> _instance;
        VkPhysicalDevice _pdevice;
        VkDevice _device;
        std::vector<VkQueue> _graphicsQueues;
        VkQueue _presentQueue;
        VkQueue _computeQueue;
        VmaAllocator allocator;
        GPU _gpu;

        Features GetEnabledFeatures();

        Device(const std::shared_ptr<Instance> &instance, GPU &gpu, Features features, uint32_t graphicsQueuesCount = 1);

        EnabledRecommendedFeatures GetEnabledRecommendedFeatures()
        {
            return enabledRecommendedFeatures;
        }

        void
        Wait();

        void Release();
    };
}