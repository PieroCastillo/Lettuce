/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_RENDER_TARGET_HPP
#define LETTUCE_CORE_RENDER_TARGET_HPP

// project headers
#include "Common.hpp"

namespace Lettuce::Core
{
    struct RenderTargetCreateInfo
    {
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        VkFormat format;
        VkComponentMapping components;
        VkImageSubresourceRange subresourceRange;
    };

    struct RenderTargetAsViewCreateInfo
    {
        VkFormat format;
        VkComponentMapping components;
        VkImageSubresourceRange subresourceRange;
    };

    class RenderTarget
    {
    private:
        VkImageLayout _layout;
        uint32_t _mipLevels, _layerCount;
        uint32_t p_width, p_height, p_depth;
        VkFormat _format;

    public:
        VkDevice m_device;
        VkImage m_image;
        VkImageView m_imageView;

        RenderTarget(VkDevice device, const RenderTargetCreateInfo& createInfo);
        RenderTarget(VkDevice device, VkImage image, const RenderTargetAsViewCreateInfo& viewInfo);
        void Release();

        VkFormat GetFormat();
    };
}
#endif // LETTUCE_CORE_RENDER_TARGET_HPP