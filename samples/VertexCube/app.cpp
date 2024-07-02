//
// Created by piero on 10/02/2024.
//
#include <iostream>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

import Lettuce;

void initWindow();
void endWindow();
void initLettuce();
void endLettuce();
void mainLoop();
void draw();

GLFWwindow *window;
const int width = 800;
const int height = 600;
Lettuce::Core::Instance instance;
Lettuce::Core::Device device;
Lettuce::Core::Swapchain swapchain;
Lettuce::Core::CommandList commandList;
Lettuce::Core::SynchronizationStructure sync;
Lettuce::Core::PipelineConnector connector;
Lettuce::Core::GraphicsPipeline graphicsPipeline;
Lettuce::Core::Shader fragmentShader;
Lettuce::Core::Shader vertexShader;
const int acquireImageSemaphoreIndex = 0;
const int renderSemaphoreIndex = 1;
const int fenceIndex = 0;

struct Vertex={
    
};

const std::string fragmentShaderText = R"(#version 320 es

precision mediump float;

layout(location = 0) in vec3 in_color;

layout(location = 0) out vec4 out_color;

void main()
{
	out_color = vec4(in_color, 1.0);
})";

const std::string vertexShaderText = R"(#version 320 es

precision mediump float;

layout(location = 0) out vec3 out_color;

vec2 triangle_positions[3] = vec2[](
    vec2(0.5, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 triangle_colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main()
{
    gl_Position = vec4(triangle_positions[gl_VertexIndex], 0.5, 1.0);

    out_color = triangle_colors[gl_VertexIndex];
}
)";

int main()
{
    initWindow();
    initLettuce();
    mainLoop();
    endLettuce();
    endWindow();
    return 0;
}

void initLettuce()
{
    instance._debug = true;
    instance.Create("Vertex Cube Sample", Lettuce::Core::Version{0, 1, 0, 0}, {});
    instance.CreateSurface(glfwGetWin32Window(window), GetModuleHandle(nullptr));
    auto gpus = instance.getGPUs();
    for (auto gpu : gpus)
    {
        std::cout << "available device: " << gpu.deviceName << std::endl;
        std::cout << "    graphics family: " << gpu.graphicsFamily.value() << std::endl;
        std::cout << "    present family : " << gpu.presentFamily.value() << std::endl;
        std::cout << "    gpu ptr:         " << gpu._pdevice << std::endl;
    }
    device.Create(instance, gpus.front(), {});
    sync.Create(device, 1, 2);
    swapchain.Create(device, sync, width, height);
    commandList.Create(device, sync);
    connector.Build(device);

    Lettuce::Core::Compilers::GLSLCompiler compiler;

    fragmentShader.Create(device, compiler, fragmentShaderText, "main", "fragment.glsl", Lettuce::Core::LettucePipelineStage::Fragment, true);
    vertexShader.Create(device, compiler, vertexShaderText, "main", "vertex.glsl", Lettuce::Core::LettucePipelineStage::Vertex, true);

    graphicsPipeline.AddShaderStage(fragmentShader);
    graphicsPipeline.AddShaderStage(vertexShader);
    graphicsPipeline.Build(device, connector, swapchain);

    fragmentShader.Destroy();
    vertexShader.Destroy();
}

void draw()
{
    sync.WaitForFence(fenceIndex);
    sync.ResetAllFences();
    swapchain.AcquireNextImage(acquireImageSemaphoreIndex);
    commandList.Reset();
    commandList.Begin();
    commandList.BeginRendering(swapchain, 0.2, 0.5, 0.3);

    commandList.BindGraphicsPipeline(graphicsPipeline);
    commandList.SetViewport(width, height);
    commandList.SetTopology(Lettuce::Core::LettuceTopology::TriangleList);
    commandList.SetScissor(swapchain);
    commandList.SetLineWidth(1.0f);
    commandList.Draw(3, 1, 0, 0);

    commandList.EndRendering(swapchain);
    commandList.End();
    commandList.Send(acquireImageSemaphoreIndex, renderSemaphoreIndex, fenceIndex);
    swapchain.Present(renderSemaphoreIndex);
    device.Wait();
}

void endLettuce()
{
    commandList.Destroy();
    graphicsPipeline.Destroy();
    connector.Destroy();
    swapchain.Destroy();
    sync.Destroy();
    device.Destroy();
    instance.Destroy();
}

void mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        draw();
    }
}

void initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(width, height, "Lettuce Vertex Cube Sample", nullptr, nullptr);
}

void endWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}