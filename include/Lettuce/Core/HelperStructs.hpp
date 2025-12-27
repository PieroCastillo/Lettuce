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
        VkPipeline m_pipeline;
        VkPipelineLayout m_layout;
    };

    struct TextureVK
    {
        VkImage m_image;
        VkImageView m_view;
    };

    struct RenderTargetVK
    {
        bool isViewOnly;
        uint32_t m_width, m_height;
        RenderTargetType m_renderTargetType;
        VkFormat m_format;
        VkImage m_image;
        VkImageView m_imageView;
    };

    struct DescriptorTableVK
    {
        VkDevice m_device;
        VkDeviceMemory m_descriptorBufferMemory;
        VkBuffer m_descriptorBuffer;
        VkPipelineLayout m_pipelineLayout;

        uint64_t m_bufferSize;
        uint64_t m_bufferAlignment;
        uint32_t m_textureDescriptorCount;
        uint32_t m_samplerDescriptorCount;

        void* m_cpuAddress;
        void* m_gpuAddress;

        void* m_pushPayload;

        std::unordered_map<VkDescriptorType, uint64_t> m_descriptorTypeSizeMap;
        std::vector<VkDescriptorSetLayout> m_setLayouts;

        // variable params
        uint64_t m_currentOffset;
    };

    struct SwapchainVK
    {
        uint32_t m_width;
        uint32_t m_height;
        std::vector<VkImage> m_swapchainImages;
        std::vector<VkImageView> m_swapchainViews;

        VkFormat m_format;
        VkSwapchainKHR m_swapchain;
        VkSurfaceKHR m_surface;
        VkFence m_waitForAcquireFence;
        uint32_t m_imageCount;
        uint32_t m_currentImageIndex;
    };
} 
#endif // LETTUCE_CORE_HELPER_STRUCTS_HPP