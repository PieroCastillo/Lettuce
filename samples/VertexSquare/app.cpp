//
// Created by piero on 10/02/2024.
// this example is made to test PushConstansts, DescriptorSets, Buffers and Indexing
//
#include <iostream>
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/vec3.hpp>
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
Lettuce::Core::Descriptor descriptor;
Lettuce::Core::PipelineConnector connector;
Lettuce::Core::GraphicsPipeline graphicsPipeline;
Lettuce::Core::Shader fragmentShader;
Lettuce::Core::Shader vertexShader;
Lettuce::Core::Buffer vertexBuffer;
Lettuce::Core::Buffer indexBuffer;
Lettuce::Core::Buffer uniformBuffer;
const int acquireImageSemaphoreIndex = 0;
const int renderSemaphoreIndex = 1;
const int fenceIndex = 0;

struct Vertex
{
    glm::vec2 inPosition;
};

struct UBO
{
    glm::mat3x3 rotation;
};

struct PushConstants{
    glm::vec3 color;
};

const std::string fragmentShaderText = R"(#version 320 es

precision mediump float;

layout(push_constant) uniform PushConstants{
    vec3 color;
} constants;
layout(location = 0) out vec4 outColor;

void main()
{
	//outColor = vec4(constants.color, 1.0);
    outColor = vec4(1.0,1.0,1.0, 1.0);
})";

const std::string vertexShaderText = R"(#version 320 es

precision mediump float;

layout(binding = 0) uniform UBO {
    mat3 rotation;
} ubo;
layout(location = 0) in vec2 inPosition;

void main()
{
    //gl_Position = vec4(ubo.rotation*vec3(inPosition,0.5), 1.0);
    gl_Position = vec4(inPosition,0.0, 1.0);
}
)";

UBO ubo;
PushConstants constants;
std::vector<Vertex> vertices = {{{-0.5f, -0.5f}}, {{0.5f, -0.5f}}, {{0.5f, 0.5f}}, {{-0.5f, 0.5f}}};
std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

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
    instance.Create("Vertex Square Sample", Lettuce::Core::Version{0, 1, 0, 0}, {});
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

    descriptor.AddDescriptorBinding(0, Lettuce::Core::DescriptorType::UniformBuffer, 1, Lettuce::Core::PipelineStage::Vertex);
    descriptor.Build(device);

    connector.AddDescriptor(descriptor);
    connector.AddPushConstant<PushConstants>(0, Lettuce::Core::PipelineStage::Fragment);
    connector.Build(device);

    Lettuce::Core::Compilers::GLSLCompiler compiler;

    fragmentShader.Create(device, compiler, fragmentShaderText, "main", "fragment.glsl", Lettuce::Core::PipelineStage::Fragment, true);
    vertexShader.Create(device, compiler, vertexShaderText, "main", "vertex.glsl", Lettuce::Core::PipelineStage::Vertex, true);

    graphicsPipeline.AddVertexBindingDescription<Vertex>(0);
    graphicsPipeline.AddVertexAttribute(0, 0, offsetof(Vertex, inPosition), (int)Lettuce::Core::Format32::Vec2F);
    graphicsPipeline.AddShaderStage(fragmentShader);
    graphicsPipeline.AddShaderStage(vertexShader);
    graphicsPipeline.Build(device, connector, swapchain, Lettuce::Core::FrontFace::CounterClockwise);

    fragmentShader.Destroy();
    vertexShader.Destroy();

    indexBuffer = Lettuce::Core::Buffer::CreateIndexBuffer(device, indices);
    vertexBuffer = Lettuce::Core::Buffer::CreateVertexBuffer(device, vertices);
    uniformBuffer = Lettuce::Core::Buffer::CreateUniformBuffer(device, &ubo);

    std::vector<Lettuce::Core::Buffer> buffers = {uniformBuffer};
    descriptor.Update<UBO>(0, buffers);
}

void update(){
    constants.color = glm::vec3(0.5,0.3,0.4);
    ubo.rotation = glm::rotate(glm::mat3x3(1.0f), glm::radians(90.0f));
}


void draw()
{
    sync.WaitForFence(fenceIndex);
    sync.ResetAllFences();
    swapchain.AcquireNextImage(acquireImageSemaphoreIndex);
    commandList.Reset();
    commandList.Begin();
    commandList.BeginRendering(swapchain, 0.2, 0.2, 0.2);

    update();

    commandList.BindDescriptorSetToGraphics(connector, descriptor);
    commandList.BindGraphicsPipeline(graphicsPipeline);
    commandList.BindVertexBuffer(vertexBuffer);
    commandList.BindIndexBuffer(indexBuffer, Lettuce::Core::IndexType::UInt16);
    commandList.PushConstant(connector, Lettuce::Core::PipelineStage::Fragment, constants);

    commandList.SetViewport(width, height);
    commandList.SetTopology(Lettuce::Core::Topology::TriangleList);
    commandList.SetScissor(swapchain);
    commandList.SetLineWidth(1.0f);

    commandList.DrawIndexed((uint32_t)indices.size());

    commandList.EndRendering(swapchain);
    commandList.End();
    commandList.Send(acquireImageSemaphoreIndex, renderSemaphoreIndex, fenceIndex);
    swapchain.Present(renderSemaphoreIndex);
    device.Wait();
}
void endLettuce()
{
    commandList.Destroy();
    indexBuffer.Destroy();
    vertexBuffer.Destroy();
    uniformBuffer.Destroy();
    graphicsPipeline.Destroy();
    connector.Destroy();
    descriptor.Destroy();
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
    window = glfwCreateWindow(width, height, "Lettuce Vertex Square Sample", nullptr, nullptr);
}

void endWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}