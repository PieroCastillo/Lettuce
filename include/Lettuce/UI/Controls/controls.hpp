/*
Created by @PieroCastillo on 2026-09-26
*/
#ifndef LETTUCE_UI_CONTROLS_HPP
#define LETTUCE_UI_CONTROLS_HPP

// standard headers
#include <any>
#include <functional>
#include <span>
#include <string>

// project headers
#include "../../Quimera/api.hpp"
#include "primitives.hpp"

namespace Lettuce::UI::Controls
{
    struct Label : public Primitives::Control
    {
        std::string text;

        void Build(Surface&, ControlInstance&) override;
        void Reset(Surface&, ControlInstance&) override;
        auto Layout(LayoutContext&) -> bool override;
        void Update(ControlInstance&, const InputState&) override;
        void Render(ControlInstance&, SurfaceCommandBuffer&) override;
    };

    struct Button : public Primitives::Control
    {
        std::string Content;
        std::function<void(std::any, bool)> Command;

        void Build(Surface&, ControlInstance&) override;
        void Reset(Surface&, ControlInstance&) override;
        auto Layout(LayoutContext&) -> bool override;
        void Update(ControlInstance&, const InputState&) override;
        void Render(ControlInstance&, SurfaceCommandBuffer&) override;
    };

    struct StackView : public Primitives::ViewControl
    {
        auto Children() const -> std::span<const Primitives::ControlRef> override;
        void Build(Surface&, ControlInstance&) override;
        void Reset(Surface&, ControlInstance&) override;
        auto Layout(LayoutContext&) -> bool override;
        void Update(ControlInstance&, const InputState&) override;
        void Render(ControlInstance&, SurfaceCommandBuffer&) override;
    };

    struct Menu : public Primitives::SelectingItemControl
    {
        void Build(Surface&, ControlInstance&) override;
        void Reset(Surface&, ControlInstance&) override;
        auto Layout(LayoutContext&) -> bool override;
        void Update(ControlInstance&, const InputState&) override;
        void Render(ControlInstance&, SurfaceCommandBuffer&) override;
        void Select(uint32_t index) override;
    };
};
#endif // LETTUCE_UI_CONTROLS_HPP