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
#include <optional>
#include <functional>

using namespace Lettuce::Core;
using namespace Lettuce::Utils;

GLFWwindow* window;

constexpr uint32_t width = 1366;
constexpr uint32_t height = 768;

Device device;
Swapchain swapchain;
DescriptorTable descriptorTable;
Pipeline rgbPipeline;
CommandAllocator cmdAlloc;
Sampler sampler;

AssetLoader loader;

Texture cubemap;

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

    AssetLoaderDesc loaderDesc = {
        .maxTempMemory = 30 * 1024 * 1024, // 30 MB
        .maxResourceMemory = 30 * 1024 * 1024, // 30 MB
    };
    loader.Create(device, loaderDesc);
}

void createRenderingObjects()
{
    // load shaders
    auto vertShader = loader.LoadSpirv("cubemap.vert.spv");
    auto fragShader = loader.LoadSpirv("cubemap.frag.spv");

    DescriptorTableDesc descriptorTableDesc = { 4,4,4 };
    descriptorTable = device.CreateDescriptorTable(descriptorTableDesc);

    std::array<Format, 1> formatArr = { device.GetRenderTargetFormat(swapchain) };
    PrimitiveShadingPipelineDesc pipelineDesc = {
        .fragmentShadingRate = false,
        .vertEntryPoint = "main",
        .fragEntryPoint = "main",
        .vertShaderBinary = vertShader,
        .fragShaderBinary = fragShader,
        .colorAttachmentFormats = std::span(formatArr),
        .descriptorTable = descriptorTable,
    };
    rgbPipeline = device.CreatePipeline(pipelineDesc);

    device.Destroy(vertShader);
    device.Destroy(fragShader);

    // create sampler
    SamplerDesc samplerDesc = {
        .magFilter = Filter::Nearest,
        .minFilter = Filter::Linear,
        .mipmap = Filter::Nearest,
        .addressModeU = SamplerAddressMode::ClampToBorder,
        .addressModeV =  SamplerAddressMode::ClampToBorder,
        .addressModeW =  SamplerAddressMode::ClampToBorder,
        .anisotropy = 2.0f
    };
    sampler = device.CreateSampler(samplerDesc);

    // load textures
    cubemap = loader.LoadKtx2Texture("../../../../samples/assets/enviroment_cubemap.ktx2");

    std::array<std::pair<uint32_t, Texture>, 1> texDescs;
    texDescs[0] = { 0, cubemap };

    std::array<std::pair<uint32_t, Sampler>, 1> samplers;
    samplers[0] = { 0, sampler };

    // push texture descriptors
    PushResourceDescriptorsDesc pushDesc = {
        .sampledTextures = std::span(texDescs),
        .samplers = std::span(samplers),
        .descriptorTable = descriptorTable,
    };
    device.PushResourceDescriptors(pushDesc);
}

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
        glfwPollEvents();
    }
}

void cleanupLettuce()
{
    device.WaitFor(QueueType::Graphics);

    device.Destroy(cubemap);

    device.Destroy(sampler);

    loader.Destroy();

    device.Destroy(rgbPipeline);
    device.Destroy(descriptorTable);

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