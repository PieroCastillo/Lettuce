// standard headers
#include <limits>
#include <memory>
#include <vector>
#include <algorithm>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/TextureDictionary.hpp"

using namespace Lettuce::Core;

void TextureDictionary::Create(const IDevice& device, const TextureDictionaryCreateInfo& createInfo)
{
    if (device.m_device == VK_NULL_HANDLE) {
        throw LettuceException(LettuceResult::InvalidDevice);
    }

    m_device = device.m_device;
    m_transferQueue = device.m_transferQueue;

    VkImageCreateInfo imageCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    };

    VkImageViewCreateInfo viewCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .components = { VK_COMPONENT_SWIZZLE_IDENTITY , VK_COMPONENT_SWIZZLE_IDENTITY , VK_COMPONENT_SWIZZLE_IDENTITY ,VK_COMPONENT_SWIZZLE_IDENTITY },
    };

    if (!haveSameSize(createInfo.formats,
        createInfo.heights,
        createInfo.isCubes,
        createInfo.layerCounts,
        createInfo.names,
        createInfo.widths))
    {
        // TODO: maybe add message?
        throw LettuceException(LettuceResult::InvalidOperation);
    }

    int imageCount = createInfo.names.size();
    m_images.reserve(imageCount);
    m_views.reserve(imageCount);
    m_names.reserve(imageCount);

    // create images & views
    for (int idx = 0; idx < imageCount; idx++)
    {
        if (createInfo.layerCounts[idx] <= 0 || createInfo.names[idx].empty())
        {
            throw LettuceException(LettuceResult::InvalidOperation);
        }
        bool isArray = createInfo.layerCounts[idx] > 1;
        imageCI.flags = (isArray ? VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT : 0) | (createInfo.isCubes[idx] ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0);
        imageCI.imageType = VK_IMAGE_TYPE_2D;
        imageCI.format = createInfo.formats[idx];
        imageCI.extent = { std::move(createInfo.widths[idx]), std::move(createInfo.heights[idx]), 1 };
        imageCI.mipLevels = createInfo.levelCounts[idx];
        imageCI.arrayLayers = createInfo.layerCounts[idx];
        imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCI.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

        VkImage image;
        handleResult(vkCreateImage(m_device, &imageCI, nullptr, &image));
        m_images.push_back(image);

        viewCI.image = image;
        viewCI.viewType = createInfo.isCubes[idx]
            ? (isArray ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE)
            : (isArray ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D);
        viewCI.format = createInfo.formats[idx];
        viewCI.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, createInfo.levelCounts[idx], 0, createInfo.layerCounts[idx]};

        VkImageView view;
        handleResult(vkCreateImageView(m_device, &viewCI, nullptr, &view));
        m_views.push_back(view);

        m_names.emplace_back(std::move(createInfo.names[idx]));
    }
}

void TextureDictionary::Release()
{
    // assume that images count equals image views count
    const uint32_t count = (uint32_t)m_images.size();
    for (uint32_t idx = 0; idx < count; idx++)
    {
        vkDestroyImageView(m_device, m_views[idx], nullptr);
        vkDestroyImage(m_device, m_images[idx], nullptr);
    }
    vkFreeMemory(m_device, m_memory, nullptr);

    m_views.clear();
    m_images.clear();
    m_names.clear();
}