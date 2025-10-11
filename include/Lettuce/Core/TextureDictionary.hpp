/*
Created by @PieroCastillo on 2025-10-10
*/
#ifndef LETTUCE_CORE_TEXTURE_DICTIONARY_HPP
#define LETTUCE_CORE_TEXTURE_DICTIONARY_HPP

// standard headers
#include <string>
#include <vector>

// project headers
#include "common.hpp"

namespace Lettuce::Core
{
    struct TextureDictionaryCreateInfo
    {
        std::vector<std::string> names;
        std::vector<uint32_t> mipMapCounts;
        std::vector<uint32_t> layerCounts;
        std::vector<VkFormat> formats;
        std::vector<uint32_t> widths;
        std::vector<uint32_t> heights;
        std::vector<bool> isArrays;
        std::vector<bool> isCubes;
    };

    class TextureDictionary
    {
    public:
        VkDevice m_device;
        VkDeviceMemory m_memory;
        std::vector<VkImage> m_images;
        std::vector<VkImageView> m_imageViews;
        std::vector<std::string> m_names;

        void Create(const std::weak_ptr<IDevice>& device, const TextureDictionaryCreateInfo& createInfo);
        void Release();
    };
};

#endif // LETTUCE_CORE_TEXTURE_DICTIONARY_HPP