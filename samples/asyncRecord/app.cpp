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

uint32_t width = 1366;
uint32_t height = 768;

Device device;
Swapchain swapchain;
DescriptorTable descriptorTable;
Pipeline rgbPipeline;

Rendering::AsyncRecorder rec;
MemoryView particlesView;

ParticleBuffer particlesData;

void initLettuce()
{
    DeviceDesc deviceCI = {
        .preferDedicated = true,
    };
    device.Create(deviceCI);

    swapchain = device.CreateSwapchain(GetSwapchainDesc(window));

    IndirectSetDesc indirectSetDesc = {
        .type = IndirectType::Draw,
        .maxCount = 128,
        .userDataSize = 0,
    };

    particlesView = device.CreateMemoryView({ sizeof(ParticleBuffer),true });
    auto pvInfo = device.GetMemoryViewInfo(particlesView);

    // move data to gpu buffer
    particlesData.idx = 0;
    particlesData.particles[0] = { { .4f,.4f,.4f}, {.4f,.4f} };
    particlesData.particles[1] = { { .7f,.7f,.7f}, {-.4f,-.4f} };

    memcpy(pvInfo.cpuAddress, &particlesData, sizeof(ParticleBuffer));

    Rendering::AsyncRecorderDesc asyncRecDesc = {
        .device = device,
        .threadCount = 4,
    };
    rec.Create(asyncRecDesc);
}

void createRenderingObjects()
{
    auto shaders = Lettuce::Utils::AssetLoader::LoadSpirv(&device, "samples/asyncRecord/asyncRecord.spv");

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
        glfwGetFramebufferSize(window, (int*)&width, (int*)&height);

        if (width == 0 || height == 0)
        {
            glfwWaitEvents();
            continue;
        }

        auto fbSize = device.NextFrame(swapchain);
        device.WaitFor(QueueType::Graphics);

        auto frame = device.GetCurrentRenderTarget(swapchain);
        BarrierDesc copyCopyBarrier[] = { {
            .srcAccess = PipelineAccess::Write,
            .srcStage = PipelineStage::Copy,
            .dstAccess = PipelineAccess::Write,
            .dstStage = PipelineStage::Copy,
        }, };

        BarrierDesc copyVertBarrier[] = { {
            .srcAccess = PipelineAccess::Write,
            .srcStage = PipelineStage::Copy,
            .dstAccess = PipelineAccess::Read,
            .dstStage = PipelineStage::VertexShader,
        }, };

        PushAllocationsDesc pushDesc;
        pushDesc.descriptorTable = descriptorTable;
        pushDesc.allocations = std::array<PushAllocationBinding, 1>{ particlesView };

        rec.Reset();
        rec.RecordAsync(std::nullopt, [&](CommandBuffer cmd, std::any _)
            {
                cmd.Fill(particlesView, 0, 0, 1);
                cmd.Barrier(copyVertBarrier);
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
                cmd.PushAllocations(pushDesc);
                cmd.Draw(3, 1);
                cmd.EndRendering();
            });
        rec.Barrier();
        rec.RecordAsync(std::nullopt, [&](CommandBuffer cmd, std::any _)
            {
                cmd.Fill(particlesView, 0, 1, 1);
                cmd.Barrier(copyVertBarrier);
                AttachmentDesc colorAttachment[1] = {
                    {
                        .renderTarget = frame,
                        .loadOp = LoadOp::Load,
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
    device.WaitFor(QueueType::Graphics);
    device.Destroy(rgbPipeline);
    device.Destroy(descriptorTable);

    device.Destroy(particlesView);

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