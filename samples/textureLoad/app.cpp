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

Texture texRgba8;
Texture texRgba32;
Texture texBC7;
Texture texB10G11R11;

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
        .maxTempMemory = 2 * 1024 * 1024, // 2 MB
        .maxResourceMemory = 2 * 1024 * 1024, // 32 MB
    };
    loader.Create(device, loaderDesc);
}

void createRenderingObjects()
{
    // load shaders
    auto shaders = loader.LoadSpirv("textureLoad.spv");

    DescriptorTableDesc descriptorTableDesc = { 4,4,4,2 };
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

    // create sampler
    SamplerDesc samplerDesc = {

    };
    sampler = device.CreateSampler(samplerDesc);

    // load textures
    texRgba8 = loader.LoadKtx2Texture("../../../../external/KTX2-Samples/ktx2/2d_rgba8_linear.ktx2");
    texRgba32 = loader.LoadKtx2Texture("../../../../external/KTX2-Samples/ktx2/2d_rgba32_linear.ktx2");
    texBC7 = loader.LoadKtx2Texture("../../../../external/KTX2-Samples/ktx2/2d_bc7.ktx2");
    texB10G11R11 = loader.LoadKtx2Texture("../../../../external/KTX2-Samples/ktx2/2d_r11g11b10_linear.ktx2");

    std::array<std::pair<uint32_t, Texture>, 4> texDescs;
    texDescs[0] = { 0, texRgba8 };
    texDescs[1] = { 1, texRgba32 };
    texDescs[2] = { 2, texBC7 };
    texDescs[3] = { 3, texB10G11R11 };

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
    device.Destroy(texRgba8);
    device.Destroy(texRgba32);
    device.Destroy(texBC7);
    device.Destroy(texB10G11R11);

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