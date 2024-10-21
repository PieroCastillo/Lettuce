//
// Created by piero on 30/09/2024.
//
#pragma once
#include <glm/glm.hpp>

namespace Lettuce::X2D::Lights
{
    class PointLight
    {
        float angleRadians;
        float intensity;
        glm::vec2 direction;
    };
}