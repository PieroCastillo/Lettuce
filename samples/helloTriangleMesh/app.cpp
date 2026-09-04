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

GLFWwindow* window;

uint32_t width = 1366;
uint32_t height = 768;

std::unique_ptr<Device> device;
Swapchain swapchain;
DescriptorTable descriptorTable;
Pipeline rgbPipeline;
CommandAllocator cmdAlloc;

void initLettuce()
{
    DeviceDesc deviceCI = {
        .preferDedicated = true,
    };
    device = std::make_unique<Device>(deviceCI);

    swapchain = device->CreateSwapchain(GetSwapchainDesc(window));

    CommandAllocatorDesc cmdAllocDesc = {
        .queueType = QueueType::Graphics,
    };
    cmdAlloc = device->CreateCommandAllocator(cmdAllocDesc);
}

void createRenderingObjects()
{
    auto shader = Lettuce::Utils::AssetLoader::LoadSpirv(device.get(), "samples/helloTriangleMesh/helloTriangleMesh.spv");

    DescriptorTableDesc descriptorTableDesc = { 4,4,4 };
    descriptorTable = device->CreateDescriptorTable(descriptorTableDesc);

    std::array<Format, 1> formatArr = { device->GetRenderTargetFormat(swapchain) };
    MeshShadingPipelineDesc pipelineDesc = {
        .fragmentShadingRate = false,
        .taskEntryPoint = "taskMain",
        .meshEntryPoint = "meshMain",
        .fragEntryPoint = "fragMain",
        .taskShaderBinary = shader,
        .meshShaderBinary = shader,
        .fragShaderBinary = shader,
        .colorAttachmentFormats = std::span(formatArr),
        .descriptorTable = descriptorTable,
    };
    rgbPipeline = device->CreatePipeline(pipelineDesc);

    device->Destroy(shader);
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

        auto fbSize = device->NextFrame(swapchain);

        device->Reset(cmdAlloc);
        auto frame = device->GetCurrentRenderTarget(swapchain);
        auto cmd = device->AllocateCommandBuffer(cmdAlloc);

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
        cmd.DrawMesh(3, 1, 1),
        cmd.EndRendering();

        std::array<std::span<CommandBuffer>, 1> cmds = { std::span(&cmd, 1) };

        CommandBufferSubmitDesc submitDesc = {
            .queueType = QueueType::Graphics,
            .commandBuffers = std::span(cmds),
            .presentSwapchain = swapchain,
        };
        device->Submit(submitDesc);

        device->DisplayFrame(swapchain);
        device->WaitFor(QueueType::Graphics);
        glfwPollEvents();
    }
}

void cleanupLettuce()
{
    device->WaitFor(QueueType::Graphics);
    device->Destroy(rgbPipeline);
    device->Destroy(descriptorTable);

    device->Destroy(cmdAlloc);
    device->Destroy(swapchain);
    device.reset();
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