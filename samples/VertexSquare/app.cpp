//
// Created by piero on 10/02/2024.
// this example is made to test PushConstansts, DescriptorSets, Buffers and Indexing
//
#include <iostream>
#include <vector>
#include <string>
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
void update();
glm::mat2x2 rotationMatrix2D(float angle);

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
Lettuce::Core::Buffer uniformBuffer2;
const int acquireImageSemaphoreIndex = 0;
const int renderSemaphoreIndex = 1;
const int fenceIndex = 0;

struct Vertex
{
    glm::vec2 inPosition;
};

struct UBO
{
    glm::mat2x2 rotation;
};

struct UBOFrag
{
    glm::vec3 color;
};

struct PushConstants
{
    glm::vec3 color;
};

const std::string fragmentShaderText = R"(#version 450

precision highp float;

layout(push_constant) uniform PushConstants{
    vec3 color;
} constants;

layout(set=0, binding=1) uniform UBOFrag{
    vec3 color;
} ubo;

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = vec4(ubo.color + constants.color, 1.0);
	//outColor = vec4(constants.color, 1.0);
  //  outColor = vec4(1.0,1.0,1.0, 1.0);
})";

const std::string vertexShaderText = R"(#version 450

layout(set = 0, binding = 0) uniform UBO {
    mat2 rotation;
} ubo;
layout(location = 0) in vec2 inPosition;

void main()
{
    vec2 t = ubo.rotation * inPosition;
    gl_Position = vec4(t.x, t.y, 0.0, 1.0);
}
)";

UBO ubo;
UBO *uboPtr = &ubo;
UBOFrag uboFrag;
UBOFrag *uboFragPtr = &uboFrag;
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

    descriptor.AddDescriptorBinding(0, Lettuce::Core::DescriptorType::UniformBuffer, Lettuce::Core::PipelineStage::Vertex);
    descriptor.AddDescriptorBinding(1, Lettuce::Core::DescriptorType::UniformBuffer, Lettuce::Core::PipelineStage::Fragment);
    descriptor.BuildLayout(device);

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
    graphicsPipeline.Build(device, connector, swapchain, Lettuce::Core::FrontFace::Clockwise);

    fragmentShader.Destroy();
    vertexShader.Destroy();

    indexBuffer = Lettuce::Core::Buffer::CreateIndexBuffer(device, indices);
    vertexBuffer = Lettuce::Core::Buffer::CreateVertexBuffer(device, vertices);

    uboPtr->rotation = rotationMatrix2D(0);
    uniformBuffer = Lettuce::Core::Buffer::CreateUniformBuffer(device, &uboPtr);

    uboFrag.color = glm::vec3(0.2f, 0.2f, 0.2f);
    uniformBuffer2 = Lettuce::Core::Buffer::CreateUniformBuffer(device, &uboFragPtr);

    descriptor.Build();

    // std::vector<Lettuce::Core::Buffer> buffers = {uniformBuffer};
    //  buffers.resize(1);
    //  buffers.push_back(uniformBuffer);
    // descriptor.Update<UBO>(0, buffers);
    descriptor.Update<UBOFrag>(1, uniformBuffer2);
    descriptor.Update<UBO>(0, uniformBuffer);
}

const float dalpha = 0.0005f;
float alpha = 0;
void update()
{
    constants.color = glm::vec3(0.6, 0.6, 0.1);
    ubo.rotation = rotationMatrix2D(alpha);
    alpha += dalpha;

    uniformBuffer.SetData((void *)uboPtr);

    // std::cout << ((*uboPtr).rotation)[0][0] << std::endl;
    // std::cout << ((*uboPtr).rotation)[1][0] << std::endl;
    // std::cout << ((*uboPtr).rotation)[0][1] << std::endl;
    // std::cout << ((*uboPtr).rotation)[1][1] << std::endl;

    uboFrag.color = glm::vec3(0.2f);
    uniformBuffer2.SetData((void *)uboFragPtr);
}

void draw()
{
    sync.WaitForFence(fenceIndex);
    swapchain.AcquireNextImage(acquireImageSemaphoreIndex);
    sync.ResetAllFences();
    commandList.Reset();
    commandList.Begin();
    commandList.BeginRendering(swapchain, 0.2, 0.2, 0.2);

    commandList.BindGraphicsPipeline(graphicsPipeline);
    commandList.PushConstant(connector, Lettuce::Core::PipelineStage::Fragment, constants);

    commandList.SetViewport(width, height);
    commandList.SetTopology(Lettuce::Core::Topology::TriangleList);
    commandList.SetScissor(swapchain);
    commandList.SetLineWidth(1.0f);

    commandList.BindVertexBuffer(vertexBuffer);
    commandList.BindIndexBuffer(indexBuffer, Lettuce::Core::IndexType::UInt16);
    commandList.BindDescriptorSetToGraphics(connector, descriptor);
    
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
    uniformBuffer2.Destroy();
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
        update();
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

glm::mat2x2 rotationMatrix2D(float angle)
{
    float cos = glm::cos(angle);
    float sin = glm::sin(angle);
    glm::mat2x2 m = {cos, sin, (-1) * sin, cos};
    return m;
}