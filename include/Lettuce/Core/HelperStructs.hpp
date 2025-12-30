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
        VkPipeline pipeline;
        VkPipelineBindPoint bindPoint;
    };

    struct MemoryHeapVK
    {
        VkDeviceMemory memory;
        uint64_t size;
        MemoryAccess access;
    };

    struct BufferVK
    {
        VkBuffer buffer;
        VkDeviceMemory memory;
        uint64_t size;
        uint64_t memoryOffset;
    };

    struct TextureVK
    {
        uint32_t width, height;
        uint32_t mipCount, layerCount;
        VkFormat format;
        VkImage image;
        VkImageView imageView;
        VkDeviceMemory memory;
        uint64_t size;
        uint64_t memoryOffset;
    };

    struct RenderTargetVK
    {
        bool isViewOnly;
        uint32_t width, height;
        VkFormat format;
        VkImage image;
        VkImageView imageView;
        VkDeviceMemory memory;
        uint64_t size;
        uint64_t memoryOffset;
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

        uint64_t  pushPayloadSize;
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

    constexpr VkFormat kFormatTable[] =
    {
        VK_FORMAT_R8G8B8A8_UNORM,          // RGBA8_UNORM
        VK_FORMAT_R8G8B8A8_SRGB,           // RGBA8_SRGB
        VK_FORMAT_B8G8R8A8_UNORM,          // BGRA8_UNORM
        VK_FORMAT_B8G8R8A8_SRGB,           // BGRA8_SRGB
        VK_FORMAT_R16G16B16A16_SFLOAT,     // RGBA16_FLOAT
        VK_FORMAT_D16_UNORM,               // D16
        VK_FORMAT_D32_SFLOAT,              // D32
        VK_FORMAT_D24_UNORM_S8_UINT,       // D24S8
        VK_FORMAT_D32_SFLOAT_S8_UINT,      // D32S8
        VK_FORMAT_R8_UNORM,                // R8
        VK_FORMAT_R8G8_UNORM,              // RG8
        VK_FORMAT_R16G16_SFLOAT            // RG16_FLOAT
    };

    constexpr VkFormat ToVkFormat(Format format)
    {
        const uint32_t index = static_cast<uint32_t>(format);
        return index < static_cast<uint32_t>(Format::COUNT)
            ? kFormatTable[index]
            : VK_FORMAT_UNDEFINED;
    }

    constexpr VkPipelineBindPoint kPipelineBindPointTable[] =
    {
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
    };

    constexpr VkPipelineBindPoint ToVkPipelineBindPoint(PipelineBindPoint bindPoint)
    {
        const uint32_t index = static_cast<uint32_t>(bindPoint);
        return index < static_cast<uint32_t>(PipelineBindPoint::Count)
            ? kPipelineBindPointTable[index]
            : VK_PIPELINE_BIND_POINT_MAX_ENUM;
    }

    constexpr VkAccessFlags2 ToVkAccess(PipelineAccess access)
    {
        return access == PipelineAccess::Read ? VK_ACCESS_MEMORY_READ_BIT : VK_ACCESS_MEMORY_WRITE_BIT;
    }

    constexpr VkPipelineStageFlags2 kPipelineStageTable[] =
    {
        VK_PIPELINE_STAGE_2_NONE,
        VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT,
        VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT,
        VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT,
        VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_2_COPY_BIT,
        VK_PIPELINE_STAGE_2_BLIT_BIT,
        VK_PIPELINE_STAGE_2_RESOLVE_BIT,
        VK_PIPELINE_STAGE_2_CLEAR_BIT,
        VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR,
        VK_PIPELINE_STAGE_2_COMMAND_PREPROCESS_BIT_EXT,
    };
    constexpr VkPipelineStageFlags2 ToVkPipelineStageFlags(PipelineStage stage)
    {
        const uint32_t index = static_cast<uint32_t>(stage);
        return index < static_cast<uint64_t>(PipelineStage::Count)
            ? kPipelineStageTable[index]
            : VK_PIPELINE_STAGE_2_NONE;
    }
}
#endif // LETTUCE_CORE_HELPER_STRUCTS_HPP