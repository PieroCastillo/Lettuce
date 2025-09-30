/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_TEXTURE_ARRAY_HPP
#define LETTUCE_CORE_TEXTURE_ARRAY_HPP

// project headers
#include "Common.hpp"

namespace Lettuce::Core
{
    struct TextureArrayCreateInfo
    {
        uint32_t width;
        uint32_t height;
        VkImageType type;
        VkImageUsageFlags imageUsage;
        uint32_t mipLevels;
        uint32_t layerCount;
        VkFormat format;
        bool isCube;
        bool isArray;
    };

    class TextureArray
    {
    private:
        uint32_t m_mipLevels, m_layerCount;
        uint32_t m_width, m_height;
        VkFormat m_format;

    public:
        VkDevice m_device;
        VkImage m_image;

        void Create(const std::weak_ptr<IDevice>& device, const TextureArrayCreateInfo& createInfo);
        void Release();

        VkFormat GetFormat();
    };
}
#endif // LETTUCE_CORE_TEXTURE_ARRAY_HPP