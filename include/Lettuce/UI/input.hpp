/*
Created by @PieroCastillo on 2026-09-26
*/
#ifndef LETTUCE_UI_INPUT_HPP
#define LETTUCE_UI_INPUT_HPP

// standard headers
#include <cstdint>
#include <bitset>

// project headers
#include "../Core/basicTypes.hpp"

using namespace Lettuce::Core;

namespace Lettuce::UI
{
    enum InputKey : uint16_t
    {
        Tab, Q, W, E, R, T, Y,
        Mayus, A, S, D, F, G, H,
        Shift, X, N, Up,
        Ctrl, Alt, Space, AltGr, Left, Down, Right,
        Count,
    };
    
    struct InputState
    {
        bool mouseLeftPressed;
        std::bitset<InputKey::Count> activeKeys;

        float2 mousePosition{};
        float2 mouseDelta{};
    };
};
#endif // LETTUCE_UI_INPUT_HPP