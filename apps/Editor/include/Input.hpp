/*
Created by @PieroCastillo on 2026-08-11
*/
#ifndef EDITOR_INPUT_HPP
#define EDITOR_INPUT_HPP

#include <chrono>
#include <cstdint>

#include "Lettuce/Lettuce.hpp"

using namespace Lettuce::Core;

namespace Editor
{
    struct InputState
    {
        bool mouseLeftPressed;

        bool wKeyPressed;
        bool aKeyPressed;
        bool sKeyPressed;
        bool dKeyPressed;

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