/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_RENDER_TARGET_HPP
#define LETTUCE_CORE_RENDER_TARGET_HPP

// standard headers
#include <optional>
#include <memory>

// project headers
#include "common.hpp"
#include "Allocators/IGPUMemoryResource.hpp"

namespace Lettuce::Core
{
    enum class RenderTargetType
    {
        ColorRGB_sRGB,
        ColorRGBA_sRGB,
        DepthStencilDS40,
    };

    struct RenderTargetCreateInfo
    {
        uint32_t width;
        uint32_t height;
        RenderTargetType type;
        bool tryCompression;
        std::shared_ptr<Allocators::IGPUMemoryResource> allocator;
    };

    class RenderTarget
    {
    private:
        friend class Swapchain;
        bool isViewOnly = false;
        uint32_t p_width, p_height;
        RenderTargetType m_renderTargetType;
        VkFormat m_format;
        std::shared_ptr<Allocators::IGPUMemoryResource> m_allocator;
        ImageAllocation m_allocation;

    public:
        VkDevice m_device;
        VkImage m_image;
        VkImageLayout m_layout;
        VkImageView m_imageView;

        void Create(const IDevice& device, const RenderTargetCreateInfo& createInfo);
        void Release();

        VkFormat GetFormat();
        RenderTargetType GetType();
    };
}
#endif // LETTUCE_CORE_RENDER_TARGET_HPP