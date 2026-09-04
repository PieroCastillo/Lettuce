#include "AppCommon.hpp"

#include <chrono>
#include <cmath>
#include <vector>
#include <filesystem>
#include <fstream>
#include <functional>
#include <numbers>
#include <optional>
#include <print>
#include <thread>

using namespace Lettuce::Core;

GLFWwindow* window;

uint32_t width = 1366;
uint32_t height = 768;

Device device;
Swapchain swapchain;
DescriptorTable descriptorTable;
Pipeline rgbPipeline;
CommandAllocator cmdAlloc;
MemoryView uniformData;
MemoryViewInfo udInfo;

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

    uniformData = device.CreateMemoryView({3 * sizeof(float), true});
    udInfo = device.GetMemoryViewInfo(uniformData);
}

void createRenderingObjects()
{
    auto shaders =Lettuce::Utils::AssetLoader::LoadSpirv(&device, "samples/uniform/uniform.spv");

    DescriptorTableDesc descriptorTableDesc = { 4,4,4 };
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
        
        PushAllocationsDesc pushDesc = {
           .allocations = std::array<PushAllocationBinding, 1>{ uniformData }, 
           .descriptorTable = descriptorTable,  
        };
        cmd.PushAllocations(pushDesc);

        // set uniform value, it could be set anytime
        timeT += delta_time();
        constexpr auto tpi = 2 * std::numbers::pi;
        ((float*)udInfo.cpuAddress)[0] = 0.5f + (0.5 * std::sin(timeT));
        ((float*)udInfo.cpuAddress)[1] = 0.5f + (0.5 * std::sin(timeT + (tpi / 3)));
        ((float*)udInfo.cpuAddress)[2] = 0.5f + (0.5 * std::sin(timeT + (2 * tpi / 3)));

        cmd.BindPipeline(rgbPipeline);
        cmd.Draw(6, 1);
        cmd.EndRendering();

        std::array<std::span<CommandBuffer>, 1> cmds = { std::span(&cmd, 1) };

        CommandBufferSubmitDesc submitDesc = {
            .queueType = QueueType::Graphics,
            .commandBuffers = std::span(cmds),
            .presentSwapchain = swapchain,
        };
        auto token = device.SubmitAsync(submitDesc);
        device.WaitFor(token);
        
        device.DisplayFrame(swapchain);

        glfwPollEvents();
    }
}

void cleanupLettuce()
{
    device.WaitFor(QueueType::Graphics);
    device.Destroy(rgbPipeline);
    device.Destroy(descriptorTable);

    device.Destroy(uniformData);
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