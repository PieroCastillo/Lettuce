//
// Created by piero on 19/02/2025.
//
#include <iostream>
#include <vector>
#include "Lettuce/Core/BufferResource.hpp"
#include "Lettuce/X3D/Geometries/Plane.hpp"
#include "Lettuce/X3D/Geometries/Primitive.hpp"

using namespace Lettuce::X3D;
using namespace Lettuce::Core;

Geometries::Plane::Plane(glm::vec3 origin, glm::vec3 gen1, glm::vec3 gen2)
{
    indices = {0, 1, 2, 1, 2, 3};
    points.reserve(4);

    points.push_back({origin,{0,0}});               // 0
    points.push_back({origin + gen1,{0,0}});        // 1
    points.push_back({origin + gen2,{0,0}});        // 2
    points.push_back({gen1 + gen2 - origin,{0,0}}); // 3

    info.vertBlock.size = points.size() * sizeof(glm::vec3);
    info.vertBlock.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    info.indexBlock.size = indices.size() * sizeof(uint32_t);
    info.indexBlock.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
}