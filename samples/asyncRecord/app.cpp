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

using namespace Lettuce;
using namespace Lettuce::Core;

GLFWwindow* window;

struct ParticleOut
{
    float color[3];
    float pos[2]; // NDC: [-1, 1]
};

struct ParticleBuffer
{
    uint32_t idx;
    ParticleOut particles[2];
};

constexpr uint32_t width = 1366;
constexpr uint32_t height = 768;

Device device;
Swapchain swapchain;
DescriptorTable descriptorTable;
Pipeline rgbPipeline;

Rendering::AsyncRecorder rec;
Allocators::LinearAllocator alloc;
MemoryView particlesView;

ParticleBuffer particlesData;

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

    IndirectSetDesc indirectSetDesc = {
        .type = IndirectType::Draw,
        .maxCount = 128,
        .userDataSize = 0,
    };

    Allocators::LinearAllocatorDesc linAllocDesc = {
        .bufferSize = sizeof(ParticleBuffer),
        .imageSize = 16,
        .cpuVisible = true,
    };
    alloc.Create(device, linAllocDesc);
    particlesView = alloc.AllocateMemory(sizeof(ParticleBuffer));

    // move data to gpu buffer
    particlesData.idx = 0;
    particlesData.particles[0] = { { .4f,.4f,.4f}, {.4f,.4f} };
    particlesData.particles[1] = { { .7f,.7f,.7f}, {-.4f,-.4f} };

    memcpy(particlesView.cpuAddress, &particlesData, sizeof(ParticleBuffer));

    Rendering::AsyncRecorderDesc asyncRecDesc = {
        .device = device,
        .threadCount = 4,
    };
    rec.Create(asyncRecDesc);
}

void createRenderingObjects()
{
    auto shadersBuffer = loadSpv("asyncRecord.spv");

    ShaderBinaryDesc shaderDesc = {
        .bytecode = std::span<uint32_t>(shadersBuffer.data(), shadersBuffer.size()),
    };
    auto shaders = device.CreateShader(shaderDesc);

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

void mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        device.NextFrame(swapchain);
        device.WaitFor(QueueType::Graphics);

        auto frame = device.GetCurrentRenderTarget(swapchain);
        BarrierDesc copyCopyBarrier[] = { {
            .srcAccess = PipelineAccess::Write,
            .srcStage = PipelineStage::Copy,
            .dstAccess = PipelineAccess::Write,
            .dstStage = PipelineStage::Copy,
        }, };

        BarrierDesc attachAttachBarrier[] = { {
            .srcAccess = PipelineAccess::Write,
            .srcStage = PipelineStage::ColorAttachmentOutput,
            .dstAccess = PipelineAccess::Write,
            .dstStage = PipelineStage::ColorAttachmentOutput,
        }, };

        BarrierDesc copyVertBarrier[] = { {
            .srcAccess = PipelineAccess::Write,
            .srcStage = PipelineStage::Copy,
            .dstAccess = PipelineAccess::Read,
            .dstStage = PipelineStage::VertexShader,
        }, };

        PushAllocationsDesc pushDesc;
        pushDesc.descriptorTable = descriptorTable;
        pushDesc.allocations = std::array{ std::pair(0u, particlesView) };

        rec.Reset();
        rec.RecordAsync(std::nullopt, [&](CommandBuffer cmd, std::any _)
            {
                cmd.Fill(particlesView, 0, 1);
                cmd.Barrier(copyVertBarrier);
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
                cmd.Draw(3, 1);
                cmd.EndRendering();
            });
        rec.Barrier();
        rec.RecordAsync(std::nullopt, [&](CommandBuffer cmd, std::any _)
            {
                cmd.Barrier(attachAttachBarrier);
                cmd.Fill(particlesView, 1, 1);
                cmd.Barrier(copyVertBarrier);
                AttachmentDesc colorAttachment[1] = {
                    {
                        .renderTarget = frame,
                        .loadOp = LoadOp::Load,
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
                cmd.Draw(3, 1);
                cmd.EndRendering();
            });

        rec.Submit(swapchain);

        device.DisplayFrame(swapchain);
        device.WaitFor(QueueType::Graphics);
        glfwPollEvents();
    }
}

void cleanupLettuce()
{
    rec.Destroy();
    alloc.Destroy();
    device.WaitFor(QueueType::Graphics);
    device.Destroy(rgbPipeline);
    device.Destroy(descriptorTable);

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