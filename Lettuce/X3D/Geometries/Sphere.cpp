//
// Created by piero on 19/02/2025.
//
#include <iostream>
#include <vector>
#include <numbers>
#include "Lettuce/Core/BufferResource.hpp"
#include "Lettuce/X3D/Geometries/Sphere.hpp"

using namespace Lettuce::X3D;
using namespace Lettuce::Core;

Geometries::Sphere::Sphere(glm::vec3 origin, float radius, int sectorCount, int stackCount)
{
    float x, y, z, xy; // vertex position
    float s, t;

    float sectorStep = 2 * std::numbers::pi_v<float> / sectorCount;
    float stackStep = std::numbers::pi_v<float> / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i)
    {
        stackAngle = std::numbers::pi / 2 - i * stackStep; // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);                    // r * cos(u)
        z = radius * sinf(stackAngle);                     // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // first and last vertices have same position and normal, but different tex coords
        for (int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep; // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle); // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle); // r * cos(u) * sin(v)

            // vertex tex coord (s, t) range between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;

            points.push_back({{x + origin.x, y + origin.y, z + origin.z}, {s, t}});
        }
    }

    int k1, k2;
    for (int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1); // beginning of current stack
        k2 = k1 + sectorCount + 1;  // beginning of next stack

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if (i != (stackCount - 1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    info.vertBlock.size = points.size() * sizeof(Vertex);
    info.vertBlock.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    info.indexBlock.size = indices.size() * sizeof(uint32_t);
    info.indexBlock.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
}