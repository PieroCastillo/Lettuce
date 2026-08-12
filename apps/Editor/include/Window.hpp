/*
Created by @PieroCastillo on 2026-08-11
*/
#ifndef EDITOR_WINDOW_HPP
#define EDITOR_WINDOW_HPP

#include <chrono>
#include <cstdint>
#include <span>
#include <string>

#include "glfw/glfw3.h"

namespace Editor
{
    class Window
    {
    private:
        GLFWwindow* window;
    public:
        Window(uint32_t width, uint32_t height, std::string title)
        {
            glfwInit();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
            window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
        }

        ~Window()
        {
            glfwDestroyWindow(window);
            glfwTerminate();
        }

        auto ShouldClose() -> bool { return  glfwWindowShouldClose(window); }
        void PollEvents() { glfwPollEvents(); }
        void WaitEvents() { glfwWaitEvents(); }
        auto GetHandle() const { return window; }

        auto GetCursorPos() const -> std::pair<double, double>
        {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            return { xpos, ypos };
        }

        auto GetSize() const -> std::pair<int, int>
        {
            int width, height;
            glfwGetFramebufferSize(window, (int*)&width, (int*)&height);
            return { width, height };
        }
    };
};
#endif // EDITOR_WINDOW_HPP