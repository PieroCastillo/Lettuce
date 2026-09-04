/*
Created by @PieroCastillo on 2026-09-03
*/
#ifndef SAMPLES_APP_COMMON_HPP
#define SAMPLES_APP_COMMON_HPP

#include "Lettuce/Lettuce.hpp"
#include "GLFW/glfw3.h"

#if defined(WIN32_) || defined(_WIN32) || defined(WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#define WIN32_LEAN_AND_MEAN
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
    return desc;
}

void InitGlfw()
{
#ifdef __linux__
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
    glfwInitHint(GLFW_WAYLAND_LIBDECOR, GLFW_WAYLAND_DISABLE_LIBDECOR);
#endif
    glfwInit();
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
}

#endif // SAMPLES_APP_COMMON_HPP