/*
Created by @PieroCastillo on 2026-09-26
*/
#ifndef LETTUCE_UI_CONTROLS_HPP
#define LETTUCE_UI_CONTROLS_HPP

// standard headers
#include <any>
#include <functional>
#include <string>

// project headers
#include "primitives.hpp"

namespace Lettuce::UI::Controls
{
    struct Label : public Primitives::Control
    {
        std::string text;

        auto Build(Surface&) -> ControlInstance override;
        void Reset(Surface&, ControlInstance&) override;
        auto Layout(ControlInstance&, float4 available) -> float4 override;
        void Update(ControlInstance&, const InputState&) override;
        void Render(ControlInstance&, SurfaceCommandBuffer&) override;
    };

    struct Button : public Primitives::Control
    {
        std::string Content;
        std::function<void(std::any, bool)> Command;
        
        auto Build(Surface&) -> ControlInstance override;
        void Reset(Surface&, ControlInstance&) override;
        auto Layout(ControlInstance&, float4 available) -> float4 override;
        void Update(ControlInstance&, const InputState&) override;
        void Render(ControlInstance&, SurfaceCommandBuffer&) override;
    };

    struct Menu : public Primitives::SelectingItemControl
    {
    };
};
#endif // LETTUCE_UI_CONTROLS_HPP