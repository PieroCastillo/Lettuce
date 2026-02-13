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
using namespace Lettuce::Rendering;

GLFWwindow* window;

constexpr uint32_t width = 1366;
constexpr uint32_t height = 768;

Device device;
Swapchain swapchain;

AsyncRecorder rec;
Allocators::LinearAllocator alloc; // I need a heap allocator uu

DescriptorTable descriptorTable;
IndirectSet isVisPass;

Pipeline pCullPass;    // compute
Pipeline pVisPass;      // mesh shading
Pipeline pPbrMaterial; // compute

constexpr uint32_t maxInstanceCount = 10000;
uint32_t instanceCount = 0;

MemoryView mvScene;
MemoryView mvInstances;
MemoryView mvMaterialDatas;
MemoryView mvIndexBuffer;
MemoryView mvVertexBuffer;
MemoryView mvMeshlets;
MemoryView mvTlasNodes;
MemoryView mvBlasNodes;
MemoryView mvBlasLeafMeshletIndices;

MemoryView mvVisibilityPassDrawcalls;

RenderTarget rtVisibilityTarget;
Texture tVisibilityTargetView; // must not be destroyed

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
        .type = IndirectType::DrawMesh,
        .maxCount = 128,
        .userDataSize = 0,
    };

    Allocators::LinearAllocatorDesc linAllocDesc = {
        .bufferSize = 10 * 1024 * 1024, // 10 MB
        .imageSize = 16,
        .cpuVisible = true,
    };
    alloc.Create(device, linAllocDesc);

    // mvScene = alloc.AllocateMemory(sizeof(SceneGPUData));
    // mvInstances = alloc.AllocateMemory(sizeof(InstanceGPUItem) * maxInstanceCount);
    // mvMaterialDatas = alloc.AllocateMemory(sizeof(MaterialDataGPUItem) * maxInstanceCount);
    // mvIndexBuffer = alloc.AllocateMemory(sizeof(uint32_t)*);
    // mvVertexBuffer = alloc.AllocateMemory(sizeof(glm::vec3) * 3 *);
    // mvMeshlets = alloc.AllocateMemory(sizeof(MeshletGPUItem)*);
    // mvTlasNodes = alloc.AllocateMemory(sizeof(TLASGPUItem)*);
    // mvBlasNodes = alloc.AllocateMemory(sizeof(BLASGPUItem)*);
    // mvBlasLeafMeshletIndices = alloc.AllocateMemory(sizeof()*);

    AsyncRecorderDesc asyncRecDesc = {
        .device = device,
        .threadCount = 4,
    };
    rec.Create(asyncRecDesc);
}

void createRenderingObjects()
{
    auto shadersBuffer = loadSpv("pbr.spv");

    ShaderBinaryDesc shaderDesc = {
        .bytecode = std::span<uint32_t>(shadersBuffer.data(), shadersBuffer.size()),
    };
    auto shaders = device.CreateShader(shaderDesc);

    DescriptorTableDesc descriptorTableDesc = { 4,4,4 };
    descriptorTable = device.CreateDescriptorTable(descriptorTableDesc);

    PushResourceDescriptorsDesc pushResDesc = {
        .storageTextures = std::array {
            std::pair(0u, tVisibilityTargetView),
        },
        .descriptorTable = descriptorTable,
    };
    device.PushResourceDescriptors(pushResDesc);

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

        BarrierDesc copyVertBarrier[] = { {
            .srcAccess = PipelineAccess::Write,
            .srcStage = PipelineStage::Copy,
            .dstAccess = PipelineAccess::Read,
            .dstStage = PipelineStage::VertexShader,
        }, };

        ExecuteIndirectDesc execDesc = {
            .indirectSet = isVisPass,
            .maxDrawCount = maxInstanceCount,
        };

        PushAllocationsDesc pushAddressesCullPassDesc = {
             .allocations = std::array {
                std::pair(0u, mvScene), // read
                std::pair(1u, mvTlasNodes),// read
                std::pair(2u, mvInstances),// read
                std::pair(3u, mvVisibilityPassDrawcalls), // write: count+drawcalls
            },
            .descriptorTable = descriptorTable,
        };

        PushAllocationsDesc pushAddressesVisibilityPassDesc = {
             .allocations = std::array {
                // task shader read
                std::pair(0u, mvScene),
                std::pair(1u, mvInstances),
                std::pair(2u, mvBlasNodes),
                std::pair(3u, mvBlasLeafMeshletIndices),
                std::pair(4u, mvMeshlets),
                // mesh shader read
                std::pair(5u, mvVertexBuffer),
                std::pair(6u, mvIndexBuffer),
                std::pair(7u, mvMeshlets),
                std::pair(8u, mvMaterialDatas),
            },
            .descriptorTable = descriptorTable,
        };

        PushAllocationsDesc pushAddressesMaterialShadingPassDesc = {
             .allocations = std::array {
                // read
                std::pair(0u, mvVertexBuffer),
                std::pair(1u, mvIndexBuffer),
                std::pair(2u, mvMaterialDatas),
                std::pair(3u, mvIndexBuffer),
            },
            .descriptorTable = descriptorTable,
        };

        AttachmentDesc visAttachment = {
            .renderTarget = rtVisibilityTarget,
            .loadOp = LoadOp::Clear,
        };
        
        RenderPassDesc visibilityRenderPassDesc = {
            .width = width,
            .height = height,
            .colorAttachments = std::span<const AttachmentDesc>(&visAttachment, 1),
            .depthStencilAttachment = std::nullopt,
        };

        rec.Reset();
        rec.RecordAsync(std::nullopt, [&](CommandBuffer cmd, std::any _)
            {
                cmd.BindDescriptorTable(descriptorTable, PipelineBindPoint::Compute);
                cmd.BindPipeline(pCullPass);
                cmd.PushAllocations(pushAddressesCullPassDesc);
                cmd.Dispatch(instanceCount / 32, 1, 1);

                cmd.BeginRendering(visibilityRenderPassDesc);
                cmd.BindDescriptorTable(descriptorTable, PipelineBindPoint::Graphics);
                cmd.BindPipeline(pVisPass);
                cmd.PushAllocations(pushAddressesVisibilityPassDesc);
                cmd.ExecuteIndirect(execDesc);
                cmd.EndRendering();

                cmd.BindDescriptorTable(descriptorTable, PipelineBindPoint::Compute);
                cmd.BindPipeline(pPbrMaterial);
                cmd.PushAllocations(pushAddressesMaterialShadingPassDesc);
                cmd.Dispatch((width * height) / 32, 1, 1);
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

    alloc.ReleaseMemory(mvBlasLeafMeshletIndices);
    alloc.ReleaseMemory(mvBlasNodes);
    alloc.ReleaseMemory(mvTlasNodes);
    alloc.ReleaseMemory(mvMeshlets);
    alloc.ReleaseMemory(mvVertexBuffer);
    alloc.ReleaseMemory(mvIndexBuffer);
    alloc.ReleaseMemory(mvMaterialDatas);
    alloc.ReleaseMemory(mvInstances);
    alloc.ReleaseMemory(mvScene);

    alloc.Destroy();
    device.WaitFor(QueueType::Graphics);
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