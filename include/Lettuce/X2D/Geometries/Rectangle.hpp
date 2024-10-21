//
// Created by piero on 30/09/2024.
//
#pragma once
#include "glm/glm.hpp"
#include "GeometryBase.hpp"
#include "Lettuce/Core/Buffer.hpp"

using namespace Lettuce::Core;

namespace Lettuce::X2D::Geometries
{
    class Rectangle : public GeometryBase
    {
    public:
        glm::mat4 transform;
        Rectangle();
        void SetSizeAndPosition(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    };
}