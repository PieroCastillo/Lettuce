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

void createRenderingObjects()
{
    auto shadersFile = std::ifstream("app.spv", std::ios::ate | std::ios::binary);
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
    };
    descriptorTable = device->CreateDescriptorTable(descriptorTableCI).value();

    GraphicsPipelineCreateData gpipelineData = {
        .shaders = shaders,
        .descriptorTable = descriptorTable,
        .colorTargets = swapchain->GetRenderViews(),
        .vertexEntryPoint = "vertexMain",
        .fragmentEntryPoint = "fragmentMain",
    };
    rgbPipeline = device->CreatePipeline(gpipelineData).value();
    shaders->Release();

    Allocators::LinearBufferSubAllocCreateInfo allocatorCI = {
        .memoryAccess = MemoryAccess::FastCPUWriteGPURead,
        .memoryUsage = AllocatorUsage::ShaderReadOnlyResource,
        .maxSize = 10 * 1024, // 10 KB    
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
}

void mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        swapchain->NextFrame();

        auto& frame = swapchain->GetCurrentRenderView();
        auto& cmd = context->GetCommandList();
        cmd.BeginRendering(width, height, { std::ref(frame) }, std::nullopt);
        cmd.BindDescriptorTable(descriptorTable);
        cmd.BindPipeline(rgbPipeline);
        cmd.Draw(3, 1);
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
    createRenderingObjects();
    mainLoop();
    cleanupLettuce();
    cleanupWindow();
    return 0;
}