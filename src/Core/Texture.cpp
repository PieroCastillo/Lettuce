// standard headers
#include <limits>
#include <memory>
#include <vector>
#include <algorithm>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Core/common.hpp"

using namespace Lettuce::Core;

Texture Device::CreateTexture(const TextureDesc& desc, const MemoryBindDesc& bindDesc)
{
    auto device = impl->m_device;

    VkFormat format = ToVkFormat(desc.format);
    
    // TODO: impl tryCompression
    VkImageSubresourceRange subresourceRange = {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,desc.mipCount,0,desc.layerCount
    };

    VkImage image;
    VkImageCreateInfo imageCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .flags = desc.isCubeMap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : (VkImageCreateFlags)0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = {desc.width, desc.height, 1},
        .mipLevels = desc.mipCount,
        .arrayLayers = desc.layerCount,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    handleResult(vkCreateImage(device, &imageCI, nullptr, &image));

    VkImageView imageView;
    VkImageViewCreateInfo viewCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = desc.isCubeMap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY},
        .subresourceRange = subresourceRange,
    };
    handleResult(vkCreateImageView(device, &viewCI, nullptr, &imageView));

    auto mem = impl->memoryHeaps.get(bindDesc.heap).memory;
    handleResult(vkBindImageMemory(device, image, mem, bindDesc.heapOffset));

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(device, image, &memReqs);

    return impl->textures.allocate({ desc.width, desc.height, desc.mipCount, desc.layerCount, format, image, imageView, mem, memReqs.size, bindDesc.heapOffset });
}

void Device::Destroy(Texture texture)
{
    if (!impl->textures.isValid(texture))
        return;

    auto device = impl->m_device;
    auto info = impl->textures.get(texture);

    vkDestroyImageView(device, info.imageView, nullptr);
    vkDestroyImage(device, info.image, nullptr);

    impl->textures.free(texture);
}