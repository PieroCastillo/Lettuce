// standard headers
#include <array>
#include <memory>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/TextureArray.hpp"

using namespace Lettuce::Core;

void TextureArray::Create(const std::weak_ptr<IDevice>& device, const TextureArrayCreateInfo& createInfo)
{
    m_device = (device.lock())->m_device;
    m_width = createInfo.width;
    m_height = createInfo.height;
    m_format = createInfo.format;
    m_layerCount = createInfo.layerCount;
    m_mipLevels = createInfo.mipLevels;

    VkImageCreateFlags imageFlags = VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;

    if (createInfo.isArray)
        imageFlags = imageFlags | VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;

    if (createInfo.isCube)
        imageFlags = imageFlags | VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    VkImageCreateInfo imageCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .flags = imageFlags,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = createInfo.format,
        .extent = { createInfo.width, createInfo.height, 1},
        .mipLevels = createInfo.mipLevels,
        .arrayLayers = createInfo.layerCount,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = createInfo.imageUsage ,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE ,
        .initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED,
    };

    handleResult(vkCreateImage(m_device, &imageCI, nullptr, &m_image));
}

void TextureArray::Release()
{
    vkDestroyImage(m_device, m_image, nullptr);
}

VkFormat TextureArray::GetFormat()
{
    return m_format;
}