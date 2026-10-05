/*
Created by @PieroCastillo on 2026-08-29
*/
#ifndef LETTUCE_UI_TYPES_HPP
#define LETTUCE_UI_TYPES_HPP

// standard headers
#include <any>
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

// project headers
#include "../Core/api.hpp"
#include "../Foundations/api.hpp"
#include "../Quimera/api.hpp"
#include "input.hpp"

namespace Lettuce::UI
{
    struct Thickness
    {
        float top, bottom, left, right;
    };

    struct MouseButtonPressedEventArgs
    {
        uint32_t x, y;
    };

    enum class VerticalAlignment
    {
        Top = 1 << 0,
        VCenter = 1 << 1,
        Bottom = 1 << 2,
    };

    enum class HorizontalAlignment
    {
        Left = 1 << 3,
        HCenter = 1 << 4,
        Right = 1 << 5,
    };

    enum State
    {
        Default,
        Focused,
        MouseHover,
        MousePressed,
    };

    class Style
    {
    public:
        virtual ~Style() = default;
        virtual auto GetBackground(State) -> Lettuce::Quimera::Brush { return {}; }
        virtual auto GetForeground(State) -> Lettuce::Quimera::Brush { return {}; }
        virtual auto GetThickness(State) -> Lettuce::Quimera::Brush { return {}; }
        virtual auto GetFontFamily() -> Lettuce::Quimera::Font { return {}; }
    };

    constexpr auto InvalidControlInstance = std::numeric_limits<uint32_t>::max();

    struct LayoutFrame;
    struct LayoutContext;
    struct ControlInstance
    {
        std::string name;

        // control
        uint32_t parent;
        uint32_t firstChild;
        uint32_t childrenCount;

        // layout
        float2 size;
        VerticalAlignment vertAligment;
        HorizontalAlignment horAlignment;
        float4 margin;
        float4 padding;
        float4 bounds; // readonly

        // style
        std::shared_ptr<Style> style;

        // interaction
        bool isEnabled;

        // functions
        std::move_only_function<void(Lettuce::Quimera::Surface&, ControlInstance&)> build = [](Lettuce::Quimera::Surface&, ControlInstance&) {};
        std::move_only_function<void(Lettuce::Quimera::Surface&, ControlInstance&)> reset = [](Lettuce::Quimera::Surface&, ControlInstance&) {};
        std::move_only_function<bool(LayoutContext&)> layout = [](LayoutContext&) { return true; };
        std::move_only_function<void(ControlInstance&, const InputState&)> update = [](ControlInstance&, const InputState&) {};
        std::move_only_function<void(ControlInstance&, Lettuce::Quimera::SurfaceCommandBuffer&)> render = [](ControlInstance&, Lettuce::Quimera::SurfaceCommandBuffer&) {};
        /* used for: render data, custom control data, etc*/
        std::any controlData;
    };

    struct LayoutFrame
    {
        ControlInstance& instance;
        float4 minMax;
        uint32_t currentChild;
    };

    struct LayoutContext
    {
        std::vector<LayoutFrame> stack;

        auto Current() -> LayoutFrame& { return stack.back(); }
        auto Self() -> ControlInstance& { return stack.back().instance; }
        void Push(ControlInstance& child, float4 constraints) { stack.emplace_back(child, constraints, 0u); }
        void Pop() { stack.pop_back(); }
    };
};
#endif // LETTUCE_UI_TYPES_HPP