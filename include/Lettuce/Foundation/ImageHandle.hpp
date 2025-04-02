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

        explicit ImageHandle(
            std::string &_name,
            std::shared_ptr<ImageResource> &_image,
            VkImageSubresourceRange _range)
            : name(_name),
              image(_image),
              range(_range)
        {
        }
    };
}