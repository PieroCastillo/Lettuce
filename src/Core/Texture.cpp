// standard headers
#include <limits>
#include <memory>
#include <vector>
#include <algorithm>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/Texture.hpp"

using namespace Lettuce::Core;

void Texture::Create(const IDevice& device, const TextureCreateInfo& createInfo)
{
    if (device.m_device == VK_NULL_HANDLE) {
        throw LettuceException(LettuceResult::InvalidDevice);
    }

    m_device = device.m_device;
    m_transferQueue = device.m_transferQueue;
    m_allocator = createInfo.allocator;

    bool isArray = createInfo.layerCount > 1;

    auto flags = (isArray ? VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT : 0) | (createInfo.isCube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0);

    m_image = createInfo.allocator->do_ialloc(
        createInfo.format,
        { std::move(createInfo.width), std::move(createInfo.height) },
        createInfo.levelCount,
        createInfo.layerCount,
        createInfo.sampled ? VK_IMAGE_USAGE_SAMPLED_BIT : VK_IMAGE_USAGE_STORAGE_BIT
    ).image;

    VkImageViewCreateInfo viewCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = m_image,
        .viewType = createInfo.isCube
            ? (isArray ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE)
            : (isArray ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D),
        .format = createInfo.format,
        .components = { VK_COMPONENT_SWIZZLE_IDENTITY , VK_COMPONENT_SWIZZLE_IDENTITY , VK_COMPONENT_SWIZZLE_IDENTITY ,VK_COMPONENT_SWIZZLE_IDENTITY },
        .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, createInfo.levelCount, 0, createInfo.layerCount},
    };
    handleResult(vkCreateImageView(m_device, &viewCI, nullptr, &m_view));
}

void Texture::Release()
{
    vkDestroyImageView(m_device, m_view, nullptr);
    m_allocator->do_ideallocate({m_image});
}