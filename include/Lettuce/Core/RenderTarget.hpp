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

namespace Lettuce::Core
{
    enum class RenderTargetType
    {
        ColorRGB,
        DepthU,
    };

    struct RenderTargetCreateInfo
    {
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        RenderTargetType type;
        bool tryCompression;
    };

    class RenderTarget
    {
    private:
        VkImageLayout _layout;
        uint32_t p_width, p_height, p_depth;
        VkFormat _format;

    public:
        VkDevice m_device;
        VkImage m_image;
        VkImageView m_imageView;

        void Create(const IDevice& device, const RenderTargetCreateInfo& createInfo);
        void Release();

        VkFormat GetFormat();
    };
}
#endif // LETTUCE_CORE_RENDER_TARGET_HPP