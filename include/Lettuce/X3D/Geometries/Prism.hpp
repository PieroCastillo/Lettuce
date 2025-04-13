//
// Created by piero on 19/02/2025.
//
#pragma once
#include "Primitive.hpp"
#include "Lettuce/Core/BufferResource.hpp"
#include <glm/glm.hpp>

using namespace Lettuce::Core;

namespace Lettuce::X3D::Geometries
{
    class Prism : public Primitive
    {
    public:
        Prism(glm::vec3 origin, glm::vec3 gen1, glm::vec3 gen2, glm::vec3 gen3);
    };
}