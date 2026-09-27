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

    struct Style
    {
        auto GetBackground(State) -> Lettuce::Quimera::Brush;
        auto GetForeground(State) -> Lettuce::Quimera::Brush;
        auto GetThickness(State) -> Lettuce::Quimera::Brush;
    };

    struct ControlInstance
    {
        std::string name;

        // control
        uint32_t parent;
        uint32_t firstChild;
        uint32_t prevSibling;
        uint32_t nextSibling;

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

        /* used for: render data, custom control data, etc*/
        std::any controlData;
    };
};
#endif // LETTUCE_UI_TYPES_HPP