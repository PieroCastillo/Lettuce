//
// Created by piero on 19/02/2025.
//
#pragma once
#include "Lettuce/Core/BufferResource.hpp"
#include <glm/glm.hpp>

using namespace Lettuce::Core;

namespace Lettuce::X3D::Geometries
{
    class Sphere
    {
    public:
        Sphere(glm::vec3 origin, float radius, int sectorCount, int stackCount);
    };
}