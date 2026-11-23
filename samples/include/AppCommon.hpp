/*
Created by @PieroCastillo on 2026-09-03
*/
#ifndef SAMPLES_APP_COMMON_HPP
#define SAMPLES_APP_COMMON_HPP

#include <print>
#include "Lettuce/Lettuce.hpp"
#include "GLFW/glfw3.h"

#if defined(WIN32_) || defined(_WIN32) || defined(WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <windows.h>
#undef CreateFont
#elifdef __linux__
#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <wayland-client.h>
#endif
#include "GLFW/glfw3native.h"

auto GetSwapchainDesc(GLFWwindow* window) -> Lettuce::Core::SwapchainDesc
{
    Lettuce::Core::SwapchainDesc desc;
    desc.clipped = true;
#if defined(WIN32_) || defined(_WIN32) || defined(WIN32)
    desc.windowPtr = glfwGetWin32Window(window);
    desc.applicationPtr = GetModuleHandle(NULL);
#elifdef __linux__
    desc.windowPtr = glfwGetWaylandWindow(window);
    desc.applicationPtr = glfwGetWaylandDisplay();
#endif
    desc.targetColor = ColorClear{{ 0.5, 0.5, 0.5, 1} };
    return desc;
}

void InitGlfw()
{
    glfwSetErrorCallback([](int error, const char* description) noexcept {
        std::println(stderr, "GLFW Error ({}): {}", error, description ? description : "No Description");
    });
#ifdef __linux__
    unsetenv("GSETTINGS_SCHEMA_DIR");
    unsetenv("GTK_PATH");
    setenv("XDG_DATA_DIRS", "/usr/local/share/:/usr/share/", 1);
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
    glfwInitHint(GLFW_WAYLAND_LIBDECOR, GLFW_WAYLAND_PREFER_LIBDECOR);
#endif
    glfwInit();
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
}

#endif // SAMPLES_APP_COMMON_HPP