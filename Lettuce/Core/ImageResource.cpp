//
// Created by piero on 29/12/2024.
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <vector>
#include "Lettuce/Core/IResource.hpp"
#include "Lettuce/Core/ImageResource.hpp"

using namespace Lettuce::Core;

ImageResource(const std::shared_ptr<Device> &device, uint32_t width, uint32_t height, uint32_t depth,
              VkImageType type,
              VkImageUsageFlags imageUsage,
              uint32_t mipLevels,
              uint32_t layerCount,
              VkFormat format,
              VkImageLayout initialLayout) : _device(device),
                                             _layout(initialLayout),
                                             _mipLevels(mipLevels),
                                             _layerCount(layerCount),
                                             _format(format),
{

    VkImageCreateInfo imageCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = type,
        .format = format,
        .extent = {width, height, depth},
        .mipLevels = mipLevels,
        .arrayLayers = layerCount,
        .samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
        .tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL,
        .usage = (VkImageUsageFlags)imageUsage,
        .sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = initialLayout,
    };

    checkResult(vkCreateImage(_device->_device, &imageCI, nullptr, &_image));
}
ResourceType ImageResource::GetResourceType()
{
    return ResourceType::Image;
}
ResourceLinearity ImageResource::GetResourceLinearity()
{
    return ResourceLinearity::NonLinear;
}
VkMemoryRequirements ImageResource::GetResourceMemoryRequirements()
{
    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(_device->_device, _image, &memReqs);
    return memReqs;
}
std::shared_ptr<IResource> ImageResource::GetReference()
{
    return std::shared_ptr<IResource>(this);
}