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
#include <optional>
#include <functional>


std::span<const uint32_t> loadSpirv(const std::string& path,
    std::vector<uint32_t>& outBuffer)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file)
        throw std::runtime_error("File not found: " + path);

    std::size_t fileSize = static_cast<std::size_t>(file.tellg());
    if (fileSize % sizeof(uint32_t) != 0)
        throw std::runtime_error("SPIR-V file size is not aligned: " + path);

    outBuffer.resize(fileSize / sizeof(uint32_t));

    file.seekg(0);
    file.read(reinterpret_cast<char*>(outBuffer.data()), fileSize);

    return std::span<const uint32_t>(outBuffer);
}

using namespace Lettuce::Core;

GLFWwindow* window;

constexpr uint32_t width = 1366;
constexpr uint32_t height = 768;

std::shared_ptr<Device> device;
std::shared_ptr<Swapchain> swapchain;
std::shared_ptr<SequentialExecutionContext> context;
std::shared_ptr<DescriptorTable> descriptorTable;
std::shared_ptr<Pipeline> rgbPipeline;

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
    std::vector<uint32_t> shaderBuffer;

    ShaderPackCreateInfo fragShaderCI = {
        .shaderByteData = loadSpirv("helloTriangleMesh.frag.spv", shaderBuffer),
    };
    auto fragShader = device->CreateShaderPack(fragShaderCI).value();

    ShaderPackCreateInfo meshShaderCI = {
      .shaderByteData = loadSpirv("helloTriangleMesh.mesh.spv", shaderBuffer),
    };
    auto meshShader = device->CreateShaderPack(meshShaderCI).value();

    ShaderPackCreateInfo taskShaderCI = {
        .shaderByteData = loadSpirv("helloTriangleMesh.task.spv", shaderBuffer),
    };
    auto taskShader = device->CreateShaderPack(taskShaderCI).value();

    DescriptorTableCreateInfo descriptorTableCI = {
        .setLayoutInfos = fragShader->GetDescriptorsInfo(), // temporal solution
        .maxDescriptorVariantsPerSet = 3,
    };
    descriptorTable = device->CreateDescriptorTable(descriptorTableCI).value();

    GraphicsPipelineCreateData gpipelineData = {
        .descriptorTable = descriptorTable,
        .colorTargets = swapchain->GetRenderViews(),
        .taskShader = std::make_tuple("main", taskShader),
        .meshShader = std::make_tuple("main", meshShader),
        .fragmentShader = std::make_tuple("main", fragShader),
    };
    rgbPipeline = device->CreatePipeline(gpipelineData).value();

    fragShader->Release();
    meshShader->Release();
    taskShader->Release();
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
        cmd.DrawMeshTasks(1, 1, 1);
        cmd.EndRendering();

        context->Execute();
        context->Wait();
        swapchain->DisplayFrame();
        glfwPollEvents();
    }
}

void cleanupLettuce()
{
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