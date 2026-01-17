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

GLFWwindow* window;

struct ParticleOut
{
    float color[3];
    float pos[2];
};

constexpr uint32_t width = 1366;
constexpr uint32_t height = 768;

Device device;
Swapchain swapchain;
DescriptorTable descriptorTable;
IndirectSet indirectSet;
Pipeline cullPipeline;
Pipeline rgbPipeline;
CommandAllocator cmdAlloc;

Allocators::LinearAllocator alloc;
MemoryView indirectView;
MemoryView particlesView;

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

    IndirectSetDesc indirectSetDesc = {
        .type = IndirectType::Draw,
        .maxCount = 128,
        .userDataSize = 0,
    };
    indirectSet = device.CreateIndirectSet(indirectSetDesc);
    indirectView = device.GetIndirectSetView(indirectSet);

    Allocators::LinearAllocatorDesc linAllocDesc = {
        .bufferSize = 128 * sizeof(ParticleOut),
        .imageSize = 16,
    };
    alloc.Create(device, linAllocDesc);
    particlesView = alloc.AllocateMemory(128 * sizeof(ParticleOut));
}

void createRenderingObjects()
{
    auto shadersBuffer = loadSpv("indirectDrawing.spv");

    ShaderBinaryDesc shaderDesc = {
        .bytecode = std::span<uint32_t>(shadersBuffer.data(), shadersBuffer.size()),
    };
    auto shaders = device.CreateShader(shaderDesc);

    DescriptorTableDesc descriptorTableDesc = { 4,4,4 };
    descriptorTable = device.CreateDescriptorTable(descriptorTableDesc);

    ComputePipelineDesc compDesc = {
        .compEntryPoint = "compMain",
        .compShaderBinary = shaders,
        .descriptorTable = descriptorTable,
    };
    cullPipeline = device.CreatePipeline(compDesc);

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
        device.NextFrame(swapchain);

        device.Reset(cmdAlloc);
        auto frame = device.GetCurrentRenderTarget(swapchain);
        auto cmd = device.AllocateCommandBuffer(cmdAlloc);

        cmd.BindDescriptorTable(descriptorTable, PipelineBindPoint::Compute);
        cmd.BindPipeline(cullPipeline);
        PushAllocationsDesc pushDesc;
        pushDesc.allocations = std::array{ std::pair(0u, indirectView),  std::pair(1u, particlesView) };
        pushDesc.descriptorTable = descriptorTable;
        cmd.PushAllocations(pushDesc);
        cmd.Dispatch(8, 1, 1);

        BarrierDesc compVertBarrier[] = { {
            .srcAccess = PipelineAccess::Write,
            .srcStage = PipelineStage::ComputeShader,
            .dstAccess = PipelineAccess::Read,
            .dstStage = PipelineStage::DrawIndirect,
        }, };
        cmd.Barrier(compVertBarrier);

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
        cmd.PushAllocations(pushDesc);
        ExecuteIndirectDesc execIndirectDesc = {
            .indirectSet = indirectSet,
            .maxDrawCount = 128,
        };
        cmd.ExecuteIndirect(execIndirectDesc);
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
    alloc.Destroy();
    device.WaitFor(QueueType::Graphics);
    device.Destroy(rgbPipeline);
    device.Destroy(cullPipeline);
    device.Destroy(descriptorTable);

    device.Destroy(indirectSet);
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