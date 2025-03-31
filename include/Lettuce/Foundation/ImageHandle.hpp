//
// Created by piero on 29/03/2025.
//
#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Lettuce/Core/ImageResource.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Foundation
{
    struct ImageHandle
    {
        std::string name;
        std::shared_ptr<ImageResource> image;
        VkImageSubresourceRange range;
    };
}