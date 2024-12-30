//
// Created by piero on 29/12/2024.
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <vector>
#include "Lettuce/Core/IResource.hpp"
#include "Lettuce/Core/ImageResource.hpp"

using namespace Lettuce::Core;

void ImageResource::Create(Device &device)
{
    VkImageCreateInfo imageCI = {

    };
    checkResult(vkCreateImage(device._device, &imageCI, nullptr, &_image));
}