//
// Created by piero on 10/02/2024.
//
#include <iostream>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "Lettuce/Lettuce.Core.hpp"

using namespace Lettuce::Core;

void initWindow();
void endWindow();
void initLettuce();
void endLettuce();
void mainLoop();
void buildCmd();
void recordCmd();
void draw();

GLFWwindow *window;
const int width = 800;
const int height = 600;
Instance instance;
Device device;
Swapchain swapchain;
// Lettuce::Core::CommandList commandList;
PipelineConnector connector;
GraphicsPipeline graphicsPipeline;
Descriptor descriptor;
Semaphore semaphore;
Shader fragmentShader;
Shader vertexShader;
uint64_t renderFinishedValue = 0;

VkCommandPool pool;
VkCommandBuffer cmd;

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
    instance.Create("TriangleSample", Lettuce::Core::Version{0, 1, 0, 0}, {});
    instance.CreateSurface(glfwGetWin32Window(window), GetModuleHandle(nullptr));
    auto gpus = instance.getGPUs();
    for (auto gpu : gpus)
    {
        std::cout << "available device: " << gpu.deviceName << std::endl;
        std::cout << "    graphics family: " << gpu.graphicsFamily.value() << std::endl;
        std::cout << "    present family : " << gpu.presentFamily.value() << std::endl;
        std::cout << "    gpu ptr:         " << gpu._pdevice << std::endl;
    }
    Features features;
    features.FragmentShadingRate = false;
    features.ConditionalRendering = false;
    device.Create(instance, gpus.front(), features);
    swapchain.Create(device, width, height);
    semaphore.Create(device, 0);

    connector.Build(device, descriptor);
    Compilers::GLSLCompiler compiler;

    fragmentShader.Create(device, compiler, fragmentShaderText, "main", "fragment.glsl", Lettuce::Core::PipelineStage::Fragment, true);
    vertexShader.Create(device, compiler, vertexShaderText, "main", "vertex.glsl", Lettuce::Core::PipelineStage::Vertex, true);

    graphicsPipeline.AddShaderStage(fragmentShader);
    graphicsPipeline.AddShaderStage(vertexShader);
    graphicsPipeline.Build(device, connector, swapchain, 0);

    fragmentShader.Destroy();
    vertexShader.Destroy();

    buildCmd();
}

void buildCmd()
{
    // rendering
    VkCommandPoolCreateInfo poolCI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = device._gpu.graphicsFamily.value(),
    };
    checkResult(vkCreateCommandPool(device._device, &poolCI, nullptr, &pool));

    VkCommandBufferAllocateInfo cmdAI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = pool,
        .level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    checkResult(vkAllocateCommandBuffers(device._device, &cmdAI, &cmd));
}

void recordCmd()
{
    // rendering cmd

    VkImageSubresourceRange imgSubresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    checkResult(vkResetCommandBuffer(cmd, 0));
    VkClearValue clearValue;
    clearValue.color = {{0.3f, 0.6f, 0.5f, 1.0f}};

    VkCommandBufferBeginInfo cmdBI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
    };
    checkResult(vkBeginCommandBuffer(cmd, &cmdBI));

    VkRect2D renderArea;
    renderArea.extent.height = swapchain.height;
    renderArea.extent.width = swapchain.width;
    renderArea.offset.x = 0;
    renderArea.offset.y = 0;

    VkRenderPassBeginInfo renderPassBI = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = swapchain._renderPass,
        .framebuffer = swapchain.framebuffers[swapchain.index],
        .renderArea = renderArea,
        .clearValueCount = 1,
        .pClearValues = &clearValue,
    };

    vkCmdBeginRenderPass(cmd, &renderPassBI, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline._pipeline);
    // line width, viewport,scissor, topology
    vkCmdSetLineWidth(cmd, 1.0f);
    VkViewport viewport = {0, 0, width, height, 0.0f, 1.0f};
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {width, height}};
    vkCmdSetScissor(cmd, 0, 1, &scissor);
    vkCmdSetPrimitiveTopology(cmd, VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    vkCmdDraw(cmd, 3, 1, 0, 0);
    vkCmdEndRenderPass(cmd);

    checkResult(vkEndCommandBuffer(cmd));
}

void draw()
{
    swapchain.AcquireNextImage();
    // sends rendering cmd
    recordCmd();

    VkCommandBufferSubmitInfo cmdSI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = cmd,
        .deviceMask = 0,
    };

    VkSemaphoreSubmitInfo signalSI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = semaphore._semaphore,
        .value = renderFinishedValue + 1,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .deviceIndex = 0,
    };

    VkSubmitInfo2 submit2I = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        // .waitSemaphoreInfoCount = 1,
        // .pWaitSemaphoreInfos = &waitSI,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &cmdSI,
        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos = &signalSI,
    };

    checkResult(vkQueueSubmit2(device._graphicsQueues[0], 1, &submit2I, VK_NULL_HANDLE));
    semaphore.Wait(renderFinishedValue + 1);
    swapchain.Present();
    renderFinishedValue++;
}

void endLettuce()
{
    vkFreeCommandBuffers(device._device, pool, 1, &cmd);
    vkDestroyCommandPool(device._device, pool, nullptr);
    semaphore.Destroy();
    graphicsPipeline.Destroy();
    connector.Destroy();
    swapchain.Destroy();
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
    window = glfwCreateWindow(width, height, "Lettuce TriangleSample", nullptr, nullptr);
}

void endWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}