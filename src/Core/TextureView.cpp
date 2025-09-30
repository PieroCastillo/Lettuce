// standard headers
#include <memory>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/TextureArray.hpp"
#include "Lettuce/Core/TextureView.hpp"

using namespace Lettuce::Core;

void TextureView::Create(const std::weak_ptr<IDevice>& device, const TextureViewCreateInfo& createInfo)
{
    m_device = (device.lock())->m_device;
    m_image = (createInfo.texture.lock())->m_image;

    VkImageViewCreateInfo imageViewCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    };

    handleResult(vkCreateImageView(m_device, &imageViewCI, nullptr, &m_imageView));
}

void TextureView::Release()
{
    vkDestroyImageView(m_device, m_imageView, nullptr);
}