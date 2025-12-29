/*
Created by @PieroCastillo on 2025-12-26
*/
#ifndef LETTUCE_CORE_HELPER_STRUCTS_HPP
#define LETTUCE_CORE_HELPER_STRUCTS_HPP

// standard headers
#include <unordered_map>
#include <vector>

// project headers
#include "api.hpp"
#include "common.hpp"
#include "HelperStructs.hpp"

namespace Lettuce::Core
{
    struct PipelineVK
    {
        VkPipeline   pipeline;
        VkPipelineLayout   layout;
    };

    struct TextureVK
    {
        VkImage   image;
        VkImageView   view;
    };

    struct RenderTargetVK
    {
        bool isViewOnly;
        uint32_t   width, height;
        RenderTargetType   renderTargetType;
        VkFormat   format;
        VkImage   image;
        VkImageView   imageView;
    };

    struct DescriptorTableVK
    {
        VkDeviceMemory descriptorBufferMemory;
        VkBuffer descriptorBuffer;
        VkDescriptorSetLayout setLayout;
        VkPipelineLayout pipelineLayout;

        uint64_t bufferSize;
        uint32_t sampledImageDescriptorCount;
        uint32_t samplerDescriptorCount;
        uint32_t storageImageDescriptorCount;

        uint64_t sampledImagesBindingOffset;
        uint64_t samplersBindingOffset;
        uint64_t storageImagesBindingOffset;

        uint64_t* cpuAddress;
        uint64_t  gpuAddress;

        uint64_t pushPayloadSize;
        uint64_t* pushPayloadAddress;
    };

    struct SwapchainVK
    {
        uint32_t width;
        uint32_t height;
        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainViews;
        std::vector<RenderTarget> renderTargets;

        VkFormat format;
        VkSwapchainKHR swapchain;
        VkSurfaceKHR surface;
        VkFence waitForAcquireFence;
        uint32_t imageCount;
        uint32_t currentImageIndex;
    };


    struct AllocationVK
    {
        uint64_t size;
        void* cpuAddress;
        uint64_t gpuAddress;
    };

    struct IndirectSetVK {};
    struct SwapchainVK {};
}
#endif // LETTUCE_CORE_HELPER_STRUCTS_HPP