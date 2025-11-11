// standard headers
#include <memory>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/RenderTarget.hpp"

using namespace Lettuce::Core;

void RenderTarget::Create(const IDevice& device, const RenderTargetCreateInfo& createInfo)
{
    m_device = device.m_device;
    m_allocator = createInfo.allocator;

    m_renderTargetType = createInfo.type;
    VkImageUsageFlags flags = 0;
    // TODO: impl tryCompression
    switch (createInfo.type)
    {
    case RenderTargetType::ColorRGB_sRGB:
        m_format = VK_FORMAT_R32G32B32_SFLOAT;
        flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        break;
    case RenderTargetType::ColorRGBA_sRGB:
        m_format = VK_FORMAT_R32G32B32A32_SFLOAT;
        flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        break;
    case RenderTargetType::DepthStencilDS40:
        m_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        flags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        break;
    }

    p_width = createInfo.width;
    p_height = createInfo.height;

    m_allocation = m_allocator->do_ialloc(m_format, { p_width, p_height }, 1, 1, flags);
    m_image = m_allocation.image;

    VkImageSubresourceRange subresourceRange = {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,1,0,1
    };

    if (m_renderTargetType == RenderTargetType::DepthStencilDS40)
    {
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    VkImageViewCreateInfo viewCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = m_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = m_format,
        .components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY},
        .subresourceRange = subresourceRange,
    };
    handleResult(vkCreateImageView(m_device, &viewCI, nullptr, &m_imageView));
}

void RenderTarget::Release()
{
    vkDestroyImageView(m_device, m_imageView, nullptr);
    m_allocator->do_ideallocate(m_allocation);
}

VkFormat RenderTarget::GetFormat()
{
    return m_format;
}

RenderTargetType RenderTarget::GetType()
{
    return m_renderTargetType;
}