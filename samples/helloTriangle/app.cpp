#include "Lettuce/Lettuce.hpp"
#include "GLFW/glfw3.h"

#if defined(WIN32_) || defined(_WIN32) || defined(WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <windows.h>
#elifdef __linux__
#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <wayland-client.h>
#endif
#include "GLFW/glfw3native.h"

#include <memory>
#include <vector>
#include <expected>
#include <thread>
#include <chrono>
#include <print>
#include <fstream>
#include <filesystem>
#include <source_location>
#include <optional>
#include <functional>

using namespace Lettuce::Core;
using namespace Lettuce::Utils;

GLFWwindow* window;

uint32_t width = 1366;
uint32_t height = 768;

Device device;
Swapchain swapchain;
DescriptorTable descriptorTable;
Pipeline rgbPipeline;
CommandAllocator cmdAlloc;

void initLettuce()
{
    DeviceDesc deviceCI = {
        .preferDedicated = true,
    };
    device.Create(deviceCI);

    SwapchainDesc swapchainDesc = {
        .clipped = true,
    };

#if defined(WIN32_) || defined(_WIN32) || defined(WIN32)
    auto appWindow = glfwGetWin32Window(window);
    auto app = GetModuleHandle(NULL);
    swapchainDesc.windowPtr = &appWindow;
    swapchainDesc.applicationPtr = &app;
#elifdef __linux__
    swapchainDesc.windowPtr = glfwGetWaylandWindow(window);
    swapchainDesc.applicationPtr = glfwGetWaylandDisplay();
#endif

    swapchain = device.CreateSwapchain(swapchainDesc);

    CommandAllocatorDesc cmdAllocDesc = {
        .queueType = QueueType::Graphics,
    };
    cmdAlloc = device.CreateCommandAllocator(cmdAllocDesc);
}

void createRenderingObjects()
{
    auto shaders = Lettuce::Utils::AssetLoader::LoadSpirv(&device, "samples/helloTriangle/helloTriangle.spv");

    DescriptorTableDesc descriptorTableDesc = { 4, 4, 4 };
    descriptorTable = device.CreateDescriptorTable(descriptorTableDesc);

    std::array<Format, 1> formatArr = { device.GetRenderTargetFormat(swapchain) };
    PrimitiveShadingPipelineDesc pipelineDesc = {
        .fragmentShadingRate = false,
        .vertEntryPoint = "vertexMain",
        .fragEntryPoint = "fragmentMain",
        .vertShaderBinary = shaders,
        .fragShaderBinary = shaders,
        .colorAttachmentFormats = std::span(formatArr),
        .descriptorTable = descriptorTable,
    };
    rgbPipeline = device.CreatePipeline(pipelineDesc);

    device.Destroy(shaders);
}

void mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwGetFramebufferSize(window, (int*)&width, (int*)&height);

        if (width == 0 || height == 0)
        {
            glfwWaitEvents();
            continue;
        }

        auto fbSize = device.NextFrame(swapchain);

        device.Reset(cmdAlloc);
        auto frame = device.GetCurrentRenderTarget(swapchain);
        auto cmd = device.AllocateCommandBuffer(cmdAlloc);

        AttachmentDesc colorAttachment[1] = {
            {
                .renderTarget = frame,
                .loadOp = LoadOp::Clear,
            } };

        RenderPassDesc renderPassDesc = {
            .width = fbSize.width,
            .height = fbSize.height,
            .colorAttachments = std::span(colorAttachment),
            .presentAttachmentIdx = 0,
        };
        cmd.BeginRendering(renderPassDesc);
        cmd.BindDescriptorTable(descriptorTable, PipelineBindPoint::Graphics);
        cmd.BindPipeline(rgbPipeline);
        cmd.Draw(3, 1);
        cmd.EndRendering();

        std::array<std::span<CommandBuffer>, 1> cmds = { std::span(&cmd, 1) };

        CommandBufferSubmitDesc submitDesc = {
            .queueType = QueueType::Graphics,
            .commandBuffers = std::span(cmds),
            .presentSwapchain = swapchain,
        };
        device.Submit(submitDesc);
        device.DisplayFrame(swapchain);
        device.WaitFor(QueueType::Graphics);

        glfwPollEvents();
    }
}

void cleanupLettuce()
{
    device.WaitFor(QueueType::Graphics);
    device.Destroy(rgbPipeline);
    device.Destroy(descriptorTable);

    device.Destroy(cmdAlloc);
    device.Destroy(swapchain);
    device.Destroy();
}

void initWindow()
{
#ifdef __linux__
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
    glfwInitHint(GLFW_WAYLAND_LIBDECOR, GLFW_WAYLAND_DISABLE_LIBDECOR);
#endif
    glfwInit();
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(width, height, "My Lettuce Window", NULL, NULL);
}

void cleanupWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

int main()
{
    std::ios::sync_with_stdio(true);
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    initWindow();
    initLettuce();
    std::println("thread count per thread group: {}", device.QueryPreferredThreadCount());
    createRenderingObjects();
    mainLoop();
    cleanupLettuce();
    cleanupWindow();
    return 0;
}