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
QueryHeap query;
Lettuce::Utils::FrameTimer timer;

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

    QueryHeapDesc queryDesc = {
        .bindPoint = PipelineBindPoint::Graphics,
        .maxQueryCount = 1,
    };
    query = device->CreateQueryHeap(queryDesc);
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
    timer.Start();
    auto accTime = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        timer.Tick();
        glfwGetFramebufferSize(window, (int*)&width, (int*)&height);

        if (width == 0 || height == 0)
        {
            glfwWaitEvents();
            continue;
        }

        auto fbSize = device->NextFrame(swapchain, width, height);

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
        cmd.ResetQueryHeap(query);
        cmd.BeginRendering(renderPassDesc);
        cmd.BindDescriptorTable(descriptorTable, PipelineBindPoint::Graphics);
        cmd.BindPipeline(rgbPipeline);
        cmd.DrawMesh(1, 1, 1, QueryRecord{ query, 0 });
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

        auto opStats = device->GetResult(query, PipelineBindPoint::Graphics, 0);
        accTime += timer.GetDeltaTime();

        if (accTime > 1.0f) {
            std::println("mesh shader invocations: {}", opStats.meshShaderInvocations);
            std::println("frag shader invocations: {}", opStats.fragmentShaderInvocations);
            std::println("last mesh shader dispatch took {:05.3f} ms", opStats.ellapsedTime * 1e-6); // ns to ms
            accTime = 0;
        }
        glfwPollEvents();
    }
}

void cleanupLettuce()
{
    device->WaitFor(QueueType::Graphics);
    device->Destroy(rgbPipeline);
    device->Destroy(descriptorTable);

    device->Destroy(query);
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