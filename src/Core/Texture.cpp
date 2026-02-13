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

[[nodiscard]] constexpr size_t alignUp(size_t v, size_t a) noexcept
{
    return (v + a - 1) / a * a;
}

Texture Device::CreateTexture(const TextureDesc& desc, const MemoryBindDesc& bindDesc)
{
    auto device = impl->m_device;

    VkImageUsageFlags usageFlags = 0;

    switch (desc.format)
    {
    case Format::Raw_R8_UInt:
    case Format::Raw_R8_SInt:
    case Format::Raw_RG8_UInt:
    case Format::Raw_RG8_SInt:
    case Format::Raw_RGBA8_UInt:
    case Format::Raw_RGBA8_SInt:
    case Format::Raw_R8_UNorm:
    case Format::Raw_R8_SNorm:
    case Format::Raw_RG8_UNorm:
    case Format::Raw_RG8_SNorm:
    case Format::Raw_RGBA8_UNorm:
    case Format::Raw_RGBA8_SNorm:
        // 16 bit component
    case Format::Raw_R16_UInt:
    case Format::Raw_R16_SInt:
    case Format::Raw_RG16_UInt:
    case Format::Raw_RG16_SInt:
    case Format::Raw_RGBA16_UInt:
    case Format::Raw_RGBA16_SInt:
    case Format::Raw_R16_UNorm:
    case Format::Raw_R16_SNorm:
    case Format::Raw_RG16_UNorm:
    case Format::Raw_RG16_SNorm:
    case Format::Raw_RGBA16_UNorm:
    case Format::Raw_RGBA16_SNorm:
    case Format::Raw_R16_SFloat:
    case Format::Raw_RG16_SFloat:
    case Format::Raw_RGBA16_SFloat:
        // 32 bit component
    case Format::Raw_R32_UInt:
    case Format::Raw_R32_SInt:
    case Format::Raw_RG32_UInt:
    case Format::Raw_RG32_SInt:
    case Format::Raw_RGBA32_UInt:
    case Format::Raw_RGBA32_SInt:
    case Format::Raw_R32_SFloat:
    case Format::Raw_RG32_SFloat:
    case Format::Raw_RGBA32_SFloat:
        usageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        break;
        // sampled/compressed or packed: 
    case Format::Compressed_BC1_RGB_UNorm:
    case Format::Compressed_BC1_RGB_sRGB:
    case Format::Compressed_BC1_RGBA_UNorm:
    case Format::Compressed_BC1_RGBA_sRGB:
    case Format::Compressed_BC2_UNorm:
    case Format::Compressed_BC2_sRGB:
    case Format::Compressed_BC3_UNorm:
    case Format::Compressed_BC3_sRGB:
    case Format::Compressed_BC4_UNorm:
    case Format::Compressed_BC4_SNorm:
    case Format::Compressed_BC5_UNorm:
    case Format::Compressed_BC5_SNorm:
    case Format::Compressed_BC6H_UFloat:
    case Format::Compressed_BC6H_SFloat:
    case Format::Compressed_BC7_UNorm:
    case Format::Compressed_BC7_sRGB:
    case Format::Raw_A2R10G10B10_UNorm_Pack32:
    case Format::Raw_A2B10G10R10_UNorm_Pack32:
    case Format::Raw_A2B10G10R10_UInt_Pack32:
    case Format::Raw_B10G11R11_UFloat_Pack32:
    case Format::Raw_E5B9G9R9_UFloat_Pack32:
    case Format::Raw_A8B8G8R8_UNorm_Pack32:
    case Format::Raw_A8B8G8R8_UInt_Pack32:
        usageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        break;
        // storage w/ atomic: 
    case Format::Atomic_R64_UInt:
    case Format::Atomic_R64_SInt:
        usageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        break;
    }

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
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED,
    };
    handleResult(vkCreateImage(device, &imageCI, nullptr, &image));

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(device, image, &memReqs);

    auto offset = alignUp(bindDesc.heapOffset, memReqs.alignment);

    auto mem = impl->memoryHeaps.get(bindDesc.heap).memory;
    handleResult(vkBindImageMemory(device, image, mem, offset));

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

    return impl->textures.allocate({ desc.width, desc.height, desc.mipCount, desc.layerCount, format, image, imageView, mem, memReqs.size, offset, false });
}

void Device::Destroy(Texture texture)
{
    if (!impl->textures.isValid(texture))
        return;

    auto device = impl->m_device;
    auto info = impl->textures.get(texture);

    // only Textures created by using Device.CreateTexture() could be destroyed
    if(info.isViewOnly)
        return;

    vkDestroyImageView(device, info.imageView, nullptr);
    vkDestroyImage(device, info.image, nullptr);

    impl->textures.free(texture);
}