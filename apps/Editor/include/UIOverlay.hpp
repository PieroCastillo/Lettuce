/*
Created by @PieroCastillo on 2026-08-12
*/
#ifndef EDITOR_UIOVERLAY_HPP
#define EDITOR_UIOVERLAY_HPP

#include <chrono>
#include <cstdint>
#include <span>
#include <string>

#include "Lettuce/Lettuce.hpp"
#include "common.hpp"
#include "Renderer.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Quimera;

namespace Editor
{
    class UIOverlay
    {
    private:
        std::unique_ptr<Surface> surface;
        Layout squareLayout;
        Layout circleLayout;
        Layout roundRectLayout;
        Geometry square;
        Geometry circle;
        Geometry roundRect;
        Brush redBrush;
        Brush blueBrush;
        Brush yellowBrush;

        void createResources();
    public:
        UIOverlay(Device& device, Swapchain swapchain);
        ~UIOverlay();

        void Record(RenderInfo&);
    };
};
#endif // EDITOR_UIOVERLAY_HPP