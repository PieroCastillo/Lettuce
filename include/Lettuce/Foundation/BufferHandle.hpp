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

        explicit BufferHandle(
            std::string &_name,
            std::shared_ptr<BufferResource> &_buffer,
            uint32_t _offset,
            uint32_t _size
            ) : name(_name),
                buffer(_buffer),
                offset(_offset),
                size(_size)
        {
        }
    };
}