// standard headers
#include <limits>
#include <memory>
#include <vector>
#include <algorithm>

// external headers
#include <volk.h>
#include <vk_mem_alloc.h>

// project headers
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Core/common.hpp"

using namespace Lettuce::Core;

// TODO: memory

[[nodiscard]] constexpr size_t alignUp(size_t v, size_t a) noexcept
{
    return (v + a - 1) / a * a;
}

auto Device::CreateTextureView(const TextureViewDesc& desc) -> TextureView
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
    VmaAllocation alloc;
    VmaAllocationInfo allocI;

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
    VmaAllocationCreateInfo allocCI = {
        .flags = desc.cpuVisible ? VMA_ALLOCATION_CREATE_MAPPED_BIT : (VmaAllocationCreateFlags)0,
        .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | (desc.cpuVisible ? VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : (VkMemoryPropertyFlags)0),
    };

    handleResult(vmaCreateImage(impl->m_allocator, &imageCI, &allocCI, &image, &alloc, &allocI));

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

    void* buffCPUAddr = nullptr;
    if (desc.cpuVisible)
        vmaMapMemory(impl->m_allocator, alloc, &buffCPUAddr);

    return impl->textures.allocate({ desc.width, desc.height, desc.mipCount, desc.layerCount,
                                    format, image, imageView, allocI.deviceMemory,
                                    allocI.size, allocI.offset, alloc, buffCPUAddr,
                                    false });
}

auto Device::CreateTextureView(const RenderTargetDesc& desc) -> TextureView
{
    VkImageUsageFlags usageFlags = 0;
    VkFormat format;
    VkImageSubresourceRange subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    switch (desc.type)
    {
    case RenderTargetType::ColorRGB_sRGB:
    {
        usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        format = VK_FORMAT_R32G32B32_SFLOAT;
        break;
    }
    case RenderTargetType::ColorRGBA_sRGB:
    {
        usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        format = VK_FORMAT_R32G32B32A32_SFLOAT;
        break;
    }
    case RenderTargetType::Depth_D32:
    {
        usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; // | VK_IMAGE_USAGE_STORAGE_BIT;
        format = VK_FORMAT_D32_SFLOAT;
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        break;
    }
    }


    VkImage image;
    VmaAllocation alloc;
    VmaAllocationInfo allocI;

    VkImageCreateInfo imageCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = {desc.width, desc.height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED,
    };
    VmaAllocationCreateInfo allocCI = {
        .flags = desc.cpuVisible ? VMA_ALLOCATION_CREATE_MAPPED_BIT : (VmaAllocationCreateFlags)0,
        .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | (desc.cpuVisible ? VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : (VkMemoryPropertyFlags)0),
    };

    handleResult(vmaCreateImage(impl->m_allocator, &imageCI, &allocCI, &image, &alloc, &allocI));

    VkImageView imageView;
    VkImageViewCreateInfo viewCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY},
        .subresourceRange = subresourceRange,
    };
    handleResult(vkCreateImageView(impl->m_device, &viewCI, nullptr, &imageView));

    void* buffCPUAddr = nullptr;
    if (desc.cpuVisible)
        vmaMapMemory(impl->m_allocator, alloc, &buffCPUAddr);

    return impl->textures.allocate({ desc.width, desc.height,1, 1,
                                    format, image, imageView, allocI.deviceMemory,
                                    allocI.size, allocI.offset, alloc, buffCPUAddr,
                                    false, ToVkClearValue(desc.defaultClearValue) });
}

void Device::Destroy(TextureView texture)
{
    if (!impl->textures.isValid(texture))
        return;

    auto info = impl->textures.get(texture);

    // only Textures created by using Device.CreateTexture() could be destroyed
    if (info.isViewOnly)
        return;

    if (info.cpuAddress)
        vmaUnmapMemory(impl->m_allocator, info.allocation);

    vkDestroyImageView(impl->m_device, info.imageView, nullptr);
    vmaDestroyImage(impl->m_allocator, info.image, info.allocation);

    impl->textures.release(texture);
}