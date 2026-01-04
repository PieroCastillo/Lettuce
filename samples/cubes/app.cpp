#include "Lettuce/Lettuce.hpp"
#include "glfw/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw/glfw3native.h"
#include "glm/glm.hpp"
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

void check(std::string_view message,
    const std::source_location& loc = std::source_location::current())
{
    std::println("[{}:{}] {}: {}",
        loc.file_name(),
        loc.line(),
        loc.function_name(),
        message);
}

using namespace Lettuce::Core;

GLFWwindow* window;

constexpr uint32_t width = 1366;
constexpr uint32_t height = 768;

struct CubeInstanceInfo
{
    glm::vec2 position;
};

struct WorldInfo
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

std::shared_ptr<Device> device;
std::shared_ptr<Swapchain> swapchain;
std::shared_ptr<SequentialExecutionContext> context;
std::shared_ptr<DescriptorTable> descriptorTable;
std::shared_ptr<Pipeline> rgbPipeline;

std::shared_ptr<Allocators::LinearBufferSubAlloc> allocator;
std::shared_ptr<DeviceVector<CubeInstanceInfo>> cubeBuffer;
std::shared_ptr<DeviceVector<WorldInfo>> worldBuffer;


std::shared_ptr<Allocators::LinearBufferSubAlloc> gAllocator;
std::shared_ptr<DeviceVector<glm::vec3>> positionsStream;
std::shared_ptr<DeviceVector<glm::vec3>> colorsStream;
std::shared_ptr<DeviceVector<uint32_t>> indicesStream;

void initLettuce()
{
    auto hwnd = glfwGetWin32Window(window);
    auto hmodule = GetModuleHandle(NULL);

    device = std::make_shared<Device>();
    DeviceCreateInfo deviceCI = {
        .preferDedicated = false,
    };
    device->Create(deviceCI);
    std::println("Device created successfully");

    SwapchainCreateInfo swapchainCI = {
        .width = width,
        .height = height,
        .clipped = true,
        .windowPtr = &hwnd,
        .applicationPtr = &hmodule,
    };
    swapchain = device->CreateSwapchain(swapchainCI).value();

    context = device->CreateSequentialContext().value();
}

void createGeometryObjects()
{
    Allocators::LinearBufferSubAllocCreateInfo allocCI = {
        .memoryAccess = MemoryAccess::FastGPUReadWrite,
        .memoryUsage = AllocatorUsage::ShaderGeometry,
        .maxSize = 2 * 1024, // 2 KB
        .alignment = 4,
    };
    // create vertex|index buffer
    gAllocator = device->CreateLinearBufferSuballocator(allocCI).value();

    allocCI.memoryAccess = MemoryAccess::FastCPUWriteGPURead;
    allocCI.memoryUsage = AllocatorUsage::Staging;
    // create staging buffer
    auto stagingAllocator = device->CreateLinearBufferSuballocator(allocCI).value();

    DeviceVectorCreateInfo dvCI = {
        .maxCount = 8,
        .allocator = gAllocator,
    };
    positionsStream = device->CreateDeviceVector<glm::vec3>(dvCI).value();
    colorsStream = device->CreateDeviceVector<glm::vec3>(dvCI).value();

    dvCI.maxCount = 36;
    indicesStream = device->CreateDeviceVector<uint32_t>(dvCI).value();

    dvCI.allocator = stagingAllocator;
    dvCI.maxCount = 8;
    auto tempPositionsStream = device->CreateDeviceVector<glm::vec3>(dvCI).value();
    auto tempColorsStream = device->CreateDeviceVector<glm::vec3>(dvCI).value();

    dvCI.maxCount = 36;
    auto tempIndicesStream = device->CreateDeviceVector<uint32_t>(dvCI).value();

    // fill & copy
    std::vector<glm::vec3> cubePositions = {
        {-0.5f, -0.5f,  0.5f},
        { 0.5f, -0.5f,  0.5f},
        { 0.5f,  0.5f,  0.5f},
        {-0.5f,  0.5f,  0.5f},

        {-0.5f, -0.5f, -0.5f},
        { 0.5f, -0.5f, -0.5f},
        { 0.5f,  0.5f, -0.5f},
        {-0.5f,  0.5f, -0.5f},
    };

    std::vector<glm::vec3> cubeColors = {
        {1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}, {1.f, 1.f, 0.f},
        {1.f, 0.f, 1.f}, {0.f, 1.f, 1.f}, {1.f, 1.f, 1.f}, {0.f, 0.f, 0.f},
    };

    std::vector<uint32_t> cubeIndices = {
        0, 1, 2,  2, 3, 0, // front
        1, 5, 6,  6, 2, 1, // right
        5, 4, 7,  7, 6, 5, // back
        4, 0, 3,  3, 7, 4, // left
        3, 2, 6,  6, 7, 3, // up
        4, 5, 1,  1, 0, 4  // down
    };

    tempColorsStream->CopyFrom(cubeColors);
    tempPositionsStream->CopyFrom(cubePositions);
    tempIndicesStream->CopyFrom(cubeIndices);

    device->MemoryCopy(tempColorsStream, colorsStream, 8);
    device->MemoryCopy(tempPositionsStream, positionsStream, 8);
    device->MemoryCopy(tempIndicesStream, indicesStream, 36);
    device->FlushCopies();

    tempPositionsStream->Release();
    tempPositionsStream->Release();
    tempIndicesStream->Release();
    stagingAllocator->Release();
}

void createRenderingObjects()
{
    auto shadersFile = std::ifstream("cubes.spv", std::ios::ate | std::ios::binary);
    if (!shadersFile) throw std::runtime_error("app.spirv does not exist");

    auto fileSize = (uint32_t)shadersFile.tellg();
    std::vector<uint32_t> shadersBuffer;
    shadersBuffer.resize(fileSize / sizeof(uint32_t));

    shadersFile.seekg(0);
    shadersFile.read((char*)shadersBuffer.data(), fileSize);

    ShaderPackCreateInfo shadersCI = {
        .shaderByteData = std::span<const uint32_t>(shadersBuffer.data(), shadersBuffer.size()),
    };
    auto shaders = device->CreateShaderPack(shadersCI).value();

    DescriptorTableCreateInfo descriptorTableCI = {
        .setLayoutInfos = shaders->GetDescriptorsInfo(),
        .maxDescriptorVariantsPerSet = 3,
        .defaultDescriptorCount = 4
    };
    descriptorTable = device->CreateDescriptorTable(descriptorTableCI).value();

    GraphicsPipelineCreateData gpipelineData = {
        .shaders = shaders,
        .descriptorTable = descriptorTable,
        .colorTargets = swapchain->GetRenderViews(),
        .inputs = { VertexInput::float32x3, VertexInput::float32x3 },
        .vertexEntryPoint = "vertexMain",
        .fragmentEntryPoint = "fragmentMain",
    };
    rgbPipeline = device->CreatePipeline(gpipelineData).value();
    shaders->Release();

    Allocators::LinearBufferSubAllocCreateInfo allocatorCI = {
        .memoryAccess = MemoryAccess::FastCPUWriteGPURead,
        .memoryUsage = AllocatorUsage::ShaderReadOnlyResource,
        .maxSize = 10 * 1024, // 10 KB
        .alignment = 4,
    };
    allocator = device->CreateLinearBufferSuballocator(allocatorCI).value();

    DeviceVectorCreateInfo dvectorCI = {
        .maxCount = 4,
        .allocator = allocator,
    };
    cubeBuffer = device->CreateDeviceVector<CubeInstanceInfo>(dvectorCI).value();

    dvectorCI.maxCount = 1;
    worldBuffer = device->CreateDeviceVector<WorldInfo>(dvectorCI).value();

    std::vector<CubeInstanceInfo> dstInfos = {
        {{0.75, 0.75}},
        {{0.75, -0.75}},
        {{-0.75, -0.75}},
        {{-0.75, 0.75}}
    };
    cubeBuffer->CopyFrom(dstInfos);

    WorldInfo worldInfo = {
        glm::mat4(),
        glm::mat4(),
        glm::mat4(),
    };
    worldBuffer->CopyFrom(std::span(&worldInfo, 1));

    auto& set = descriptorTable->CreateSetInstance("shaderInput", "inputs");
    set.Register("shaderInput.worldInfo", { worldBuffer->GetHandle() });
    set.Register("shaderInput.cubeInstances", { cubeBuffer->GetHandle() });

    descriptorTable->BuildSets();
}

void mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        swapchain->NextFrame();

        auto& frame = swapchain->GetCurrentRenderView();
        auto& cmd = context->GetCommandList();
        cmd.BeginRendering(width, height, { std::ref(frame) }, std::nullopt);
        cmd.BindIndexStream(indicesStream->GetHandle());
        cmd.BindVertexStreams({ positionsStream->GetHandle(), colorsStream->GetHandle() });
        cmd.BindDescriptorTable(descriptorTable);
        cmd.BindPipeline(rgbPipeline);
        cmd.DrawIndexed(36, 1);
        cmd.EndRendering();

        context->Execute();
        context->Wait();
        swapchain->DisplayFrame();
        glfwPollEvents();
    }
}

void cleanupLettuce()
{
    worldBuffer->Release();
    cubeBuffer->Release();
    allocator->Release();

    rgbPipeline->Release();
    descriptorTable->Release();

    indicesStream->Release();
    colorsStream->Release();
    positionsStream->Release();
    gAllocator->Release();

    context->Release();
    swapchain->Release();
    device->Release();
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
    createGeometryObjects();
    createRenderingObjects();
    mainLoop();
    cleanupLettuce();
    cleanupWindow();
    return 0;
}