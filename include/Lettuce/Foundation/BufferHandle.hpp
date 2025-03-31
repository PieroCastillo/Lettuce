//
// Created by piero on 29/03/2025.
//
#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Lettuce/Core/BufferResource.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Foundation
{
    struct BufferHandle
    {
        std::string name;
        std::shared_ptr<BufferResource> buffer;
        uint32_t offset;
        uint32_t size;
    };
}