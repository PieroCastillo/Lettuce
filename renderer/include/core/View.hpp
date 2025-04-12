//
// Created by piero on 11/04/2025.
//
#pragma once
#include <vector>
#include <memory>
#include <variant>
#include <string>
#include <utility>
#include <vector>

#include "Lettuce/Lettuce.X3D.hpp"

using namespace Lettuce::X3D;

namespace Lettuce::Renderer::Core
{
    struct View
    {
        Camera3D camera;
        Scene scene;

        uint32_t width, height, x, y;
    };
}