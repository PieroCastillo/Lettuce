//
// Created by piero on 14/02/2024.
//
#pragma once

#include <iostream>
#include <set>
#include <vector>
#include "Instance.hpp"
#include "GPU.hpp"

namespace Lettuce::Core
{
    struct Features
    {
        bool FragmentShadingRate;
        bool PresentWait;
        bool ExecutionGraphs;
        bool MeshShading;
        bool RayTracing;
        bool Video;
        bool MemoryBudget;
        bool ConditionalRendering;
        bool DescriptorBuffer;
    };

    class Device
    {
    private:
        std::vector<char *> availableExtensionsNames;
        std::vector<char *> availableLayersNames;
        std::vector<const char *> requestedExtensionsNames;
        std::vector<const char *> requestedLayersNames;
        Features _features;
        Features _enabledFeatures;

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
        VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragmentShadingRateFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR,
            .pNext = &gpuFeatures13,
        };
        VkPhysicalDevicePresentWaitFeaturesKHR presentWaitFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR,
            .pNext = &fragmentShadingRateFeature,
        };
        VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
            .pNext = &presentWaitFeature,
        };
        VkPhysicalDeviceConditionalRenderingFeaturesEXT conditionalRenderingFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT,
            .pNext = &meshShaderFeature,
        };
        VkPhysicalDeviceDescriptorBufferFeaturesEXT descriptorBufferFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT,
            .pNext = &conditionalRenderingFeature,
        };

        bool tryAddFeatureAndExt(const char *extName);

        void createFeaturesChain();

        void listExtensions();

        void listLayers();

        void loadExtensionsLayersAndFeatures();

    public:
        Instance _instance;
        VkPhysicalDevice _pdevice;
        VkDevice _device;
        std::vector<VkQueue> _graphicsQueues;
        VkQueue _presentQueue;
        VmaAllocator allocator;
        GPU _gpu;

        Features GetEnabledFeatures();

        void Create(Instance &instance, GPU &gpu, Features features, uint32_t graphicsQueuesCount = 1);

        void Wait();

        void Destroy();
    };
}