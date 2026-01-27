/*
Created by @PieroCastillo on 2025-12-26
*/
#ifndef LETTUCE_CORE_HELPER_STRUCTS_HPP
#define LETTUCE_CORE_HELPER_STRUCTS_HPP

// standard headers
#include <unordered_map>
#include <vector>
#include <type_traits>

// project headers
#include "api.hpp"
#include "common.hpp"

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
        void* baseCpuAddress;
    };

    struct BufferVK
    {
        VkBuffer buffer;
        VkDeviceMemory memory;
        uint64_t size;
        uint64_t memoryOffset;

        void* cpuAddress;
        uint64_t gpuAddress;
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
        VkClearValue defaultClearValue;
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

        uint8_t* cpuAddress;
        uint64_t  gpuAddress;
    };

    struct SwapchainVK
    {
        uint32_t width;
        uint32_t height;
        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainViews;
        std::vector<RenderTarget> renderTargets;

        Format ltFormat;
        VkFormat format;
        VkSwapchainKHR swapchain;
        VkSurfaceKHR surface;
        VkFence waitForAcquireFence;
        std::vector<VkSemaphore> presentSemaphores;
        uint32_t imageCount;
        uint32_t currentImageIndex;
    };

    struct CommandAllocatorVK
    {
        VkCommandPool pool;
    };

    struct AllocationVK
    {
        uint64_t size;
        void* cpuAddress;
        uint64_t gpuAddress;
    };

    struct IndirectSetVK
    {
        VkDeviceMemory indirectSetMemory;
        VkBuffer indirectSetBuffer;
        IndirectType type;
        uint32_t stride;

        uint32_t size;
        uint64_t gpuAddress;

        // if VK_EXT_device_generated_commands is enable
        VkIndirectCommandsLayoutEXT commandsLayout;
        VkIndirectExecutionSetEXT executionSet;
    };

    constexpr std::array<std::uint8_t, 64> kFormatTable = {
        9, 10, 13, 14, 16, 17, 20, 21,
        37, 38, 41, 42,
        44, 50, 51, 55,
        58, 64, 68,
        70, 71, 74, 75, 76, 77, 78,
        81, 82, 83,
        91, 92, 95, 96, 97,
        98, 99, 100,
        101, 102, 103,
        107, 108, 109,
        110, 111,
        122, 123,
        130,
        131, 132, 133, 134,
        135, 136, 137, 138,
        139, 140, 141, 142,
        143, 144, 145, 146
    };

    consteval std::array<bool, 256> make_format_lut() {
        std::array<bool, 256> lut{};
        for (auto f : kFormatTable)
        {
            lut[f] = true;
        }
        return lut;
    }
    constexpr auto format_lut = make_format_lut();
    constexpr bool is_valid_format_value(std::uint8_t v) noexcept {
        return format_lut[v];
    }

    constexpr VkFormat ToVkFormat(Format format)
    {
        return static_cast<VkFormat>(format);
    }

    constexpr Format FromVkFormat(VkFormat vkFormat)
    {
        if(!is_valid_format_value(vkFormat))
            return Format::Undefined;

        return static_cast<Format>(vkFormat);
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

    constexpr VkAttachmentLoadOp kLoadOpTable[] =
    {
        VK_ATTACHMENT_LOAD_OP_LOAD,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_LOAD_OP_NONE,
    };
    constexpr VkAttachmentLoadOp ToVkAttachmentLoadOp(LoadOp loadOp)
    {
        const uint32_t index = static_cast<uint32_t>(loadOp);
        return index < static_cast<uint64_t>(LoadOp::Count)
            ? kLoadOpTable[index]
            : VK_ATTACHMENT_LOAD_OP_NONE;
    };

    inline VkClearValue ToVkClearValue(const ClearValue& clear)
    {
        return std::visit(
            [](const auto& value) -> VkClearValue {
                VkClearValue vk{};

                using T = std::decay_t<decltype(value)>;

                if constexpr (std::is_same_v<T, ColorClear>) {
                    vk.color.float32[0] = value.value[0];
                    vk.color.float32[1] = value.value[1];
                    vk.color.float32[2] = value.value[2];
                    vk.color.float32[3] = value.value[3];
                }
                else if constexpr (std::is_same_v<T, DepthStencilClear>) {
                    vk.depthStencil.depth = value.depth;
                    vk.depthStencil.stencil = value.stencil;
                }

                return vk;
            },
            clear
        );
    }

    constexpr VkFilter ToVkFilter(Filter f) {
        return f == Filter::Linear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
    }

    constexpr VkSamplerMipmapMode ToVkMipmapMode(Filter m) {
        return m == Filter::Linear
            ? VK_SAMPLER_MIPMAP_MODE_LINEAR
            : VK_SAMPLER_MIPMAP_MODE_NEAREST;
    }

    constexpr VkSamplerAddressMode ToVkAddressMode(SamplerAddressMode m) {
        switch (m) {
        case SamplerAddressMode::Repeat:          return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case SamplerAddressMode::ClampToEdge:     return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case SamplerAddressMode::ClampToBorder:   return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case SamplerAddressMode::MirroredRepeat:  return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        }
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
}
#endif // LETTUCE_CORE_HELPER_STRUCTS_HPP