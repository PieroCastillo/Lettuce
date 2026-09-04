#include "GLFW/glfw3.h"

#include "Input.hpp"

void Editor::InputSystem::Update(GLFWwindow* window)
{
    state.mouseLeftPressed = GLFW_PRESS == glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

    state.activeKeys[InputKey::Tab] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_TAB);
    state.activeKeys[InputKey::Q] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_Q);
    state.activeKeys[InputKey::W] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_W);
    state.activeKeys[InputKey::E] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_E);
    state.activeKeys[InputKey::R] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_R);
    state.activeKeys[InputKey::T] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_T);
    state.activeKeys[InputKey::Y] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_Y);
    state.activeKeys[InputKey::Mayus] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_CAPS_LOCK);
    state.activeKeys[InputKey::A] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_A);
    state.activeKeys[InputKey::S] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_S);
    state.activeKeys[InputKey::D] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_D);
    state.activeKeys[InputKey::F] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_F);
    state.activeKeys[InputKey::G] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_G);
    state.activeKeys[InputKey::H] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_H);
    state.activeKeys[InputKey::Shift] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
    state.activeKeys[InputKey::X] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_X);
    state.activeKeys[InputKey::N] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_N);
    state.activeKeys[InputKey::Up] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_UP);
    state.activeKeys[InputKey::Ctrl] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_LEFT_CONTROL);
    state.activeKeys[InputKey::Alt] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_LEFT_ALT);
    state.activeKeys[InputKey::Space] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_A);
    state.activeKeys[InputKey::AltGr] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_RIGHT_ALT);
    state.activeKeys[InputKey::Left] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_LEFT);
    state.activeKeys[InputKey::Down] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_DOWN);
    state.activeKeys[InputKey::Right] = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_RIGHT);

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    auto oldPos = state.mousePosition;
    state.mousePosition = float2(xpos, ypos);
    state.mouseDelta = state.mousePosition - oldPos;
}