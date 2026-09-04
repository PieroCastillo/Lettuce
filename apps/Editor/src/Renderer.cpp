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

#include "Renderer.hpp"

using namespace Editor;

Renderer::Renderer(Window& window)
{
    DeviceDesc deviceCI = {
        .preferDedicated = true,
    };
    device = std::make_unique<Device>(deviceCI);

    SwapchainDesc swapchainDesc = {
        .clipped = true,
    };
#if defined(WIN32_) || defined(_WIN32) || defined(WIN32)
    swapchainDesc.windowPtr = glfwGetWin32Window(window.GetHandle());
    swapchainDesc.applicationPtr = GetModuleHandle(NULL);
#elifdef __linux__
    swapchainDesc.windowPtr = glfwGetWaylandWindow(window.GetHandle());
    swapchainDesc.applicationPtr = glfwGetWaylandDisplay();
#endif
    swapchain = device->CreateSwapchain(swapchainDesc);

    CommandAllocatorDesc cmdAllocDesc = {
        .queueType = QueueType::Graphics,
    };
    cmdAlloc = device->CreateCommandAllocator(cmdAllocDesc);

    auto [width, height] = window.GetSize();
    Resize(width, height);
}

Renderer::~Renderer()
{
    device->WaitFor(QueueType::Graphics);

    device->Destroy(depthTarget);

    device->Destroy(cmdAlloc);
    device->Destroy(swapchain);
    device.reset();
}

void Renderer::BeginFrame()
{

}

void Renderer::EndFrame()
{

}

void Renderer::Resize(uint32_t width, uint32_t height)
{
    device->WaitFor(QueueType::Graphics);
    if (depthTarget.generation != 0)
        device->Destroy(depthTarget);

    RenderTargetDesc depthDesc = {
        .width = width,
        .height = height,
        .type = RenderTargetType::Depth_D32,
        .defaultClearValue = DepthStencilClear {1.0f, 0},
    };
    depthTarget = device->CreateTextureView(depthDesc);
}