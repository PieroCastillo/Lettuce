//
// Created by piero on 19/02/2025.
//
#pragma once
#include "Lettuce/Core/BufferResource.hpp"
#include <glm/glm.hpp>

using namespace Lettuce::Core;

namespace Lettuce::X3D
{
    struct PrimitiveInfo
    {
        BufferBlock vertBlock;
        BufferBlock indexBlock;
    };

    class Primitive
    {
    public:
        PrimitiveInfo info;
        std::vector<glm::vec3> points;
        std::vector<uint32_t> indices;
        void Clear()
        {
            points.clear();
            points.shrink_to_fit();
            indices.clear();
            indices.shrink_to_fit();
        }
    };
}