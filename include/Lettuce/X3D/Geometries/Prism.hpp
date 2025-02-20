//
// Created by piero on 19/02/2025.
//
#pragma once
#include "Lettuce/Core/BufferResource.hpp"
#include <glm/glm.hpp>

using namespace Lettuce::Core;

namespace Lettuce::X3D
{
    class Prism
    {
    public:
        BufferBlock vertBlock;
        BufferBlock indexBlock;
        Prism(glm::vec3 origin, glm::vec3 gen1, glm::vec3 gen2, glm::vec3 gen3);
    };
}