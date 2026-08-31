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
};
#endif // LETTUCE_UI_TYPES_HPP