//
// Created by piero on 19/02/2025.
//
#include <iostream>
#include <vector>
#include "Lettuce/Core/BufferResource.hpp"
#include "Lettuce/X3D/Geometries/Prism.hpp"

using namespace Lettuce::X3D;
using namespace Lettuce::Core;

Prism::Prism(glm::vec3 origin, glm::vec3 gen1, glm::vec3 gen2, glm::vec3 gen3)
{
    std::vector<glm::vec3> points;
    std::vector<uint32_t> indices = {
        0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 0, 1, 5, 0, 4, 5, 2, 3, 6, 3, 6, 7, 1, 2, 5, 2, 5, 6, 0, 3, 4, 3, 4, 7};
    points.reserve(8);

    points.push_back(origin);                      // 0
    points.push_back(origin + gen1);               // 1
    points.push_back(gen3 + gen1 - origin);        // 2
    points.push_back(origin + gen3);               // 3
    points.push_back(origin + gen2);               // 4
    points.push_back(gen1 + gen2 - origin);        // 5
    points.push_back(gen3 + gen2 + gen1 - origin); // 6
    points.push_back(gen3 + gen2 - origin);        // 7

    BufferBlock vertBlock;
    BufferBlock indexBlock;
    vertBlock.size = points.size() * sizeof(glm::vec3);
    vertBlock.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    indexBlock.size = indices.size() * sizeof(uint32_t);
    indexBlock.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
}