/*
Created by @PieroCastillo on 2026-08-11
*/
#ifndef EDITOR_COMMON_HPP
#define EDITOR_COMMON_HPP

#include <chrono>
#include <cstdint>
#include <span>
#include <string>

#include "Lettuce/Lettuce.hpp"
#include "Window.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Rendering;

namespace Editor
{
    struct RenderInfo
    {
        CommandBuffer& cmd;
        TextureView frame;
        TextureView depthTarget;
        TextureView pickTarget;
        uint32_t fbWidth;
        uint32_t fbHeight;
    };
};
#endif // EDITOR_COMMON_HPP