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
#include "Allocators/IGPUMemoryResource.hpp"

namespace Lettuce::Core
{
    struct TextureCreateInfo
    {
        uint32_t width;
        uint32_t height;
        VkFormat format;
        uint32_t layerCount;
        uint32_t levelCount;
        bool isCube;
        bool sampled;
        std::shared_ptr<Allocators::IGPUMemoryResource> allocator;
    };

    class Texture
    {
        VkDevice m_device;
        VkQueue m_transferQueue;
        VkImage m_image;
        VkImageView m_view;
        std::shared_ptr<Allocators::IGPUMemoryResource> m_allocator;
    public:
        void Create(const IDevice& device, const TextureCreateInfo& createInfo);
        void Release();

        inline uint64_t GetImageHandle() { return (uint64_t)m_image; }
        inline uint64_t GetImageViewHandle() { return (uint64_t)m_view; }
    };
};
#endif // LETTUCE_CORE_TEXTURE_DICTIONARY_HPP