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
#include <cmath>
#include <numbers>

using namespace Lettuce::Core;

GLFWwindow* window;

constexpr uint32_t width = 1366;
constexpr uint32_t height = 768;

Device device;
Swapchain swapchain;
DescriptorTable descriptorTable;
Pipeline rgbPipeline;
CommandAllocator cmdAlloc;
Allocators::LinearAllocator linAllocator;
MemoryView uniformData;

std::vector<uint32_t> loadSpv(std::string path)
{
    auto shadersFile = std::ifstream(path, std::ios::ate | std::ios::binary);
    if (!shadersFile) throw std::runtime_error(path + " does not exist");

    auto fileSize = (uint32_t)shadersFile.tellg();
    std::vector<uint32_t> shadersBuffer;
    shadersBuffer.resize(fileSize / sizeof(uint32_t));

    shadersFile.seekg(0);
    shadersFile.read((char*)shadersBuffer.data(), fileSize);

    return shadersBuffer;
}

void initLettuce()
{
    auto hwnd = glfwGetWin32Window(window);
    auto hmodule = GetModuleHandle(NULL);

    DeviceDesc deviceCI = {
        .preferDedicated = true,
    };
    device.Create(deviceCI);

    SwapchainDesc swapchainDesc = {
        .width = width,
        .height = height,
        .clipped = true,
        .windowPtr = &hwnd,
        .applicationPtr = &hmodule,
    };
    swapchain = device.CreateSwapchain(swapchainDesc);

    CommandAllocatorDesc cmdAllocDesc = {
        .queueType = QueueType::Graphics,
    };
    cmdAlloc = device.CreateCommandAllocator(cmdAllocDesc);

    Allocators::LinearAllocatorDesc linAllocDesc = {
        .bufferSize = 4096,
        .imageSize = 16,
        .cpuVisible = true,
    };
    linAllocator.Create(device, linAllocDesc);
    uniformData = linAllocator.AllocateMemory(3 * sizeof(float));

    std::println("uniform gpu address: ", uniformData.gpuAddress);
}

void createRenderingObjects()
{
    auto shadersBuffer = loadSpv("uniform.spv");

    ShaderBinaryDesc shaderDesc = {
        .bytecode = std::span<uint32_t>(shadersBuffer.data(), shadersBuffer.size()),
    };
    auto shaders = device.CreateShader(shaderDesc);

    DescriptorTableDesc descriptorTableDesc = { 4,4,4,2 };
    descriptorTable = device.CreateDescriptorTable(descriptorTableDesc);

    device.PushAllocations(descriptorTable, std::array{ std::pair(0u, uniformData) });

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

double delta_time()
{
    static auto last = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();

    std::chrono::duration<double> dt = now - last;
    last = now;

    return dt.count();
}
double timeT = 0;

void mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        device.NextFrame(swapchain);

        device.Reset(cmdAlloc);
        auto frame = device.GetCurrentRenderTarget(swapchain);
        auto cmd = device.AllocateCommandBuffer(cmdAlloc);

        AttachmentDesc colorAttachment[1] = {
            {
                .renderTarget = frame,
                .loadOp = LoadOp::Clear,
            }
        };

        RenderPassDesc renderPassDesc = {
            .width = width,
            .height = height,
            .colorAttachments = std::span(colorAttachment),
        };
        cmd.BeginRendering(renderPassDesc);
        cmd.BindDescriptorTable(descriptorTable, PipelineBindPoint::Graphics);

        // set uniform value, it could be set anytime
        timeT += delta_time();
        constexpr auto tpi = 2 * std::numbers::pi;
        ((float*)uniformData.cpuAddress)[0] = 0.5f + (0.5 * std::sin(timeT));
        ((float*)uniformData.cpuAddress)[1] = 0.5f + (0.5 * std::sin(timeT + (tpi / 3)));
        ((float*)uniformData.cpuAddress)[2] = 0.5f + (0.5 * std::sin(timeT + (2 * tpi / 3)));

        cmd.BindPipeline(rgbPipeline);
        cmd.Draw(6, 1);
        cmd.EndRendering();

        std::array<std::span<CommandBuffer>, 1> cmds = { std::span(&cmd, 1) };

        CommandBufferSubmitDesc submitDesc = {
            .queueType = QueueType::Graphics,
            .commandBuffers = std::span(cmds),
            .presentSwapchain = swapchain,
        };
        device.Submit(submitDesc);

        device.DisplayFrame(swapchain);
        glfwPollEvents();
    }
}

void cleanupLettuce()
{
    device.Destroy(rgbPipeline);
    device.Destroy(descriptorTable);

    linAllocator.Destroy();
    device.Destroy(cmdAlloc);
    device.Destroy(swapchain);
    device.Destroy();
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