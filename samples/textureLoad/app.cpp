#include "AppCommon.hpp"

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
Sampler sampler;

AssetLoader loader;

TextureView texRgba8;
TextureView texRgba32;
TextureView texBC7;
TextureView texB10G11R11;

void initLettuce()
{
    DeviceDesc deviceCI = {
        .preferDedicated = true,
    };
    device.Create(deviceCI);

    swapchain = device.CreateSwapchain(GetSwapchainDesc(window));

    CommandAllocatorDesc cmdAllocDesc = {
        .queueType = QueueType::Graphics,
    };
    cmdAlloc = device.CreateCommandAllocator(cmdAllocDesc);
}

void createRenderingObjects()
{
    // load shaders
    auto shader = AssetLoader::LoadSpirv(&device, "./samples/textureLoad/textureLoad.spv");

    DescriptorTableDesc descriptorTableDesc = { 4,4,4 };
    descriptorTable = device.CreateDescriptorTable(descriptorTableDesc);

    std::array<Format, 1> formatArr = { device.GetRenderTargetFormat(swapchain) };
    PrimitiveShadingPipelineDesc pipelineDesc = {
        .fragmentShadingRate = false,
        .vertEntryPoint = "vertMain",
        .fragEntryPoint = "fragMain",
        .vertShaderBinary = shader,
        .fragShaderBinary = shader,
        .colorAttachmentFormats = std::span(formatArr),
        .descriptorTable = descriptorTable,
    };
    rgbPipeline = device.CreatePipeline(pipelineDesc);

    device.Destroy(shader);

    // create sampler
    SamplerDesc samplerDesc = {
        .magFilter = Filter::Nearest,
        .minFilter = Filter::Linear,
        .mipmap = Filter::Nearest,
        .addressModeU = SamplerAddressMode::ClampToBorder,
        .addressModeV = SamplerAddressMode::ClampToBorder,
        .addressModeW = SamplerAddressMode::ClampToBorder,
        .anisotropy = 2.0f
    };
    sampler = device.CreateSampler(samplerDesc);

    // load textures
    auto copyCmd = device.CreateCommandAllocator({QueueType::Copy});
    texRgba8 = AssetLoader::LoadKtx2Texture(&device, copyCmd, "../../../../external/KTX2-Samples/ktx2/2d_rgba8_linear.ktx2");
    texRgba32 = AssetLoader::LoadKtx2Texture(&device, copyCmd, "../../../../external/KTX2-Samples/ktx2/2d_rgba32_linear.ktx2");
    texBC7 = AssetLoader::LoadKtx2Texture(&device, copyCmd, "../../../../external/KTX2-Samples/ktx2/2d_bc7.ktx2");
    texB10G11R11 = AssetLoader::LoadKtx2Texture(&device, copyCmd, "../../../../external/KTX2-Samples/ktx2/2d_r11g11b10_linear.ktx2");
    device.Destroy(copyCmd);

    std::array<std::pair<uint32_t, TextureView>, 4> texDescs;
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
            }
        };

        RenderPassDesc renderPassDesc = {
            .width = fbSize.width,
            .height = fbSize.height,
            .colorAttachments = std::span(colorAttachment),
            .presentAttachmentIdx = 0,
        };
        cmd.BeginRendering(renderPassDesc);
        cmd.BindDescriptorTable(descriptorTable, PipelineBindPoint::Graphics);
        cmd.BindPipeline(rgbPipeline);
        cmd.Draw(6, 4);
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

    device.Destroy(texRgba8);
    device.Destroy(texRgba32);
    device.Destroy(texBC7);
    device.Destroy(texB10G11R11);

    device.Destroy(sampler);

    device.Destroy(rgbPipeline);
    device.Destroy(descriptorTable);

    device.Destroy(cmdAlloc);
    device.Destroy(swapchain);
    device.Destroy();
}

void initWindow()
{
    InitGlfw();
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