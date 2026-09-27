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
    };

    struct Button : public Primitives::ContentControl
    {
        std::string Content;
        std::function<void(std::any, bool)> command;
    };

    struct Menu : public Primitives::SelectingItemControl
    {
    };
};
#endif // LETTUCE_UI_CONTROLS_HPP