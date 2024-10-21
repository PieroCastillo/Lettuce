//
// Created by piero on 30/09/2024.
//
#pragma once
#include "Lettuce/Core/Buffer.hpp"

using namespace Lettuce::Core;

namespace Lettuce::X2D::Geometries
{
    class GeometryBase
    {
    public:
        Buffer indexBuffer;
        Buffer vertexBuffer;
    };
}