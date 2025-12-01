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
#include <optional>
#include <functional>

using namespace Lettuce::Core;

GLFWwindow* window;

constexpr uint32_t width = 1366;
constexpr uint32_t height = 768;

std::shared_ptr<Device> device;
std::shared_ptr<Swapchain> swapchain;
std::shared_ptr<SequentialExecutionContext> context;
std::shared_ptr<DescriptorTable> descriptorTable;
std::shared_ptr<Pipeline> rgbPipeline;

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

    context = device->CreateSequentialContext().value();
}

void createRenderingObjects()
{
    auto shadersFile = std::ifstream("helloTriangleMesh.spv", std::ios::ate | std::ios::binary);
    if (!shadersFile) throw std::runtime_error("helloTriangleMesh.spv does not exist");

    auto fileSize = (uint32_t)shadersFile.tellg();
    std::vector<uint32_t> shadersBuffer;
    shadersBuffer.resize(fileSize / sizeof(uint32_t));

    shadersFile.seekg(0);
    shadersFile.read((char*)shadersBuffer.data(), fileSize);

    ShaderPackCreateInfo shadersCI = {
        .shaderByteData = std::span<const uint32_t>(shadersBuffer.data(), shadersBuffer.size()),
    };
    auto shaders = device->CreateShaderPack(shadersCI).value();

    DescriptorTableCreateInfo descriptorTableCI = {
        .setLayoutInfos = shaders->GetDescriptorsInfo(),
        .maxDescriptorVariantsPerSet = 3,
    };
    descriptorTable = device->CreateDescriptorTable(descriptorTableCI).value();

    GraphicsPipelineCreateData gpipelineData = {
        .shaders = shaders,
        .descriptorTable = descriptorTable,
        .colorTargets = swapchain->GetRenderViews(),
        //.taskEntryPoint = "amplificationMain",
        .meshEntryPoint = "meshMain",
        .fragmentEntryPoint = "fragmentMain",
    };
    rgbPipeline = device->CreatePipeline(gpipelineData).value();

    shaders->Release();
}

void mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        swapchain->NextFrame();

        auto& frame = swapchain->GetCurrentRenderView();
        auto& cmd = context->GetCommandList();
        cmd.BeginRendering(width, height, { std::ref(frame) }, std::nullopt);
        cmd.BindDescriptorTable(descriptorTable);
        cmd.BindPipeline(rgbPipeline);
        cmd.DrawMeshTasks(1, 1, 1);
        cmd.EndRendering();

        context->Execute();
        context->Wait();
        swapchain->DisplayFrame();
        glfwPollEvents();
    }
}

void cleanupLettuce()
{
    rgbPipeline->Release();
    descriptorTable->Release();

    context->Release();
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
    std::ios::sync_with_stdio(true);
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    initWindow();
    initLettuce();
    createRenderingObjects();
    mainLoop();
    cleanupLettuce();
    cleanupWindow();
    return 0;
}