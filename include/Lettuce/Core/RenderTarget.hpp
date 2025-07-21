/*
Creted by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_RENDER_TARGET_HPP
#define LETTUCE_CORE_RENDER_TARGET_HPP

namespace Lettuce::Core
{
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

        RenderTarget(VkDevice device, uint32_t width, uint32_t height, uint32_t depth,
                     VkImageType type,
                     VkImageUsageFlags imageUsage,
                     uint32_t mipLevels,
                     uint32_t layerCount,
                     VkFormat format,
                     VkImageLayout initialLayout);
        RenderTarget(VkDevice device, VkImage customImage,
                     uint32_t width, uint32_t height, uint32_t depth,
                     VkImageType type,
                     VkImageUsageFlags imageUsage,
                     uint32_t mipLevels,
                     uint32_t layerCount,
                     VkFormat format,
                     VkImageLayout initialLayout);
        void Release();

        VkFormat GetFormat();
    };
}
#endif // LETTUCE_CORE_RENDER_TARGET_HPP