#include "Lettuce/Lettuce.hpp"
#include "glfw/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw/glfw3native.h"
#include <windows.h>

#include <memory>
#include <vector>
#include <expected>
#include <thread>
#include <chrono>
#include <print>
#include <fstream>
#include <filesystem>
#include <source_location>

void check(std::string_view message,
    const std::source_location& loc = std::source_location::current())
{
    std::println("[{}:{}] {}: {}",
        loc.file_name(),
        loc.line(),
        loc.function_name(),
        message);
}


using namespace Lettuce::Core;

GLFWwindow* window;

constexpr uint32_t width = 1366;
constexpr uint32_t height = 768;

std::shared_ptr<Device> device;
std::shared_ptr<Swapchain> swapchain;
std::shared_ptr<Pipeline> rgbPipeline;

std::shared_ptr<RenderFlowGraph<CommandRecordingContext>> renderGraph;

void initLettuce()
{
    auto hwnd = glfwGetWin32Window(window);
    auto hmodule = GetModuleHandle(NULL);

    device = std::make_shared<Device>();
    DeviceCreateInfo deviceCI = {
        .preferDedicated = false,
    };
    device->Create(deviceCI);
    std::println("Device created successfully");

    SwapchainCreateInfo swapchainCI = {
        .width = width,
        .height = height,
        .clipped = true,
        .windowPtr = &hwnd,
        .applicationPtr = &hmodule,
    };
    swapchain = device->CreateSwapchain(swapchainCI).value();
}

void createRenderingObjects()
{
    auto shadersFile = std::ifstream("app.spv", std::ios::ate | std::ios::binary);
    if (!shadersFile) throw std::runtime_error("app.spirv does not exist");

    auto fileSize = (uint32_t)shadersFile.tellg();
    std::vector<uint32_t> shadersBuffer;
    shadersBuffer.resize(fileSize / sizeof(uint32_t));

    shadersFile.seekg(0);
    shadersFile.read((char*)shadersBuffer.data(), fileSize);

    ShaderPackCreateInfo shadersCI = {
        .shaderByteData = std::span<const uint32_t>(shadersBuffer.data(), shadersBuffer.size()),
    };
    auto shaders = device->CreateShaderPack(shadersCI).value();

    shaders->Release();
}

void createRenderGraph()
{
    renderGraph = device->CreateGraph<CommandRecordingContext>().value();
    auto node = renderGraph->CreateNode<CommandRecordingContext>(NodeKind::Graphics, [&](const CommandRecordingContext& ctx) {

        });
    renderGraph->Compile();
}

void mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        swapchain->NextFrame();

        swapchain->DisplayFrame();
        glfwPollEvents();
    }
}

void cleanupLettuce()
{
    renderGraph->Release();

    swapchain->Release();
    device->Release();
}

void initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(width, height, "My Lettuce Window", NULL, NULL);
}

void cleanupWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

int main()
{
    initWindow();
    initLettuce();
    createRenderingObjects();
    createRenderGraph();
    mainLoop();
    cleanupLettuce();
    cleanupWindow();
    return 0;
}