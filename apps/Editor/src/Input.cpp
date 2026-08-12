#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw/glfw3.h"
#include "glfw/glfw3native.h"
#include <windows.h>

#include "Input.hpp"

void Editor::InputSystem::Update(GLFWwindow* window)
{
    state.mouseLeftPressed = GLFW_PRESS == glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

    state.aKeyPressed = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_A) || GLFW_PRESS == glfwGetKey(window, GLFW_KEY_LEFT);
    state.wKeyPressed = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_W) || GLFW_PRESS == glfwGetKey(window, GLFW_KEY_UP);
    state.sKeyPressed = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_S) || GLFW_PRESS == glfwGetKey(window, GLFW_KEY_DOWN);
    state.dKeyPressed = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_D) || GLFW_PRESS == glfwGetKey(window, GLFW_KEY_RIGHT);

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    auto oldPos = state.mousePosition;
    state.mousePosition = float2(xpos, ypos);
    state.mouseDelta = state.mousePosition - oldPos;
}