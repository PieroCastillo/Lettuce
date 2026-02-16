// standard headers
#include <memory>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Core/common.hpp"

using namespace Lettuce::Core;

RenderTarget Device::CreateRenderTarget(const RenderTargetDesc& desc, const MemoryBindDesc& bindDesc)
{
    auto device = impl->m_device;

    VkFormat format;
    VkImageUsageFlags usage = 0;
    // TODO: impl tryCompression
    // TODO: impl MSAA
    switch (desc.type)
    {
    case RenderTargetType::ColorRGB_sRGB:
        format = VK_FORMAT_R32G32B32_SFLOAT;
        usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        break;
    case RenderTargetType::ColorRGBA_sRGB:
        format = VK_FORMAT_R32G32B32A32_SFLOAT;
        usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        break;
    case RenderTargetType::DepthStencilDS40:
        format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        break;
    }

    VkImageSubresourceRange subresourceRange = {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,1,0,1
    };

    VkImage image;
    VkImageCreateInfo imageCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = {desc.width, desc.height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    handleResult(vkCreateImage(device, &imageCI, nullptr, &image));

    if (desc.type == RenderTargetType::DepthStencilDS40)
    {
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    VkImageView imageView;
    VkImageViewCreateInfo viewCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY},
        .subresourceRange = subresourceRange,
    };
    handleResult(vkCreateImageView(device, &viewCI, nullptr, &imageView));

    auto mem = impl->memoryHeaps.get(bindDesc.heap).memory;
    handleResult(vkBindImageMemory(device, image, mem, bindDesc.heapOffset));

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(device, image, &memReqs);

    auto texView = impl->textures.allocate({ desc.width, desc.height, 1, 1, format, image, imageView, mem, memReqs.size, bindDesc.heapOffset, true });
    return impl->renderTargets.allocate({ false, desc.width, desc.height, format, image, imageView, mem, memReqs.size, bindDesc.heapOffset, ToVkClearValue(desc.defaultClearValue), texView });
}

void Device::Destroy(RenderTarget renderTarget)
{
    if (!impl->renderTargets.isValid(renderTarget))
        return;

    auto device = impl->m_device;
    auto info = impl->renderTargets.get(renderTarget);

    if (info.isViewOnly)
        return;

    vkDestroyImageView(device, info.imageView, nullptr);
    vkDestroyImage(device, info.image, nullptr);

    impl->textures.free(info.texView);
    impl->renderTargets.free(renderTarget);
}

Texture Device::GetTextureView(RenderTarget renderTarget)
{
    auto device = impl->m_device;
    auto info = impl->renderTargets.get(renderTarget);

    return info.texView;
}