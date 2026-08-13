/*
Created by @PieroCastillo on 2026-08-11
*/
#ifndef EDITOR_INPUT_HPP
#define EDITOR_INPUT_HPP

#include <chrono>
#include <cstdint>
#include <bitset>

#include "Lettuce/Lettuce.hpp"

using namespace Lettuce::Core;

namespace Editor
{
    enum InputKey : uint16_t
    {
        Tab, Q, W, E, R, T, Y,
        Mayus, A, S, D, F, G, H,
        Shift, X, N, Up,
        Ctrl, Alt, Space, AltGr, Left, Down, Right,
        Count,
    };

    constexpr InputKey operator|(InputKey a, InputKey b)
    {
        return InputKey(uint16_t(a) | uint16_t(b));
    }

    constexpr InputKey operator&(InputKey a, InputKey b)
    {
        return InputKey(uint16_t(a) & uint16_t(b));
    }

    struct InputState
    {
        bool mouseLeftPressed;
        std::bitset<InputKey::Count> activeKeys;

        float2 mousePosition{};
        float2 mouseDelta{};
    };

    class InputSystem
    {
        InputState state;
    public:
        void Update(GLFWwindow* window);

        auto GetState() const { return state; };
    };
};
#endif // EDITOR_INPUT_HPP