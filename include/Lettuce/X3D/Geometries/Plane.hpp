//
// Created by piero on 19/02/2025.
//
#pragma once
#include "Lettuce/Core/BufferResource.hpp"
#include <glm/glm.hpp>

using namespace Lettuce::Core;

namespace Lettuce::X3D::Geometries
{
    class Plane
    {
    public:
        Plane(glm::vec3 origin, glm::vec3 gen1, glm::vec3 gen2);
    };
}