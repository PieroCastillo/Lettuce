//
// Created by piero on 08/02/2024.
//
#include <iostream>
#include <vector>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <cmath>
#include <numbers>

#include "Lettuce/Lettuce.Core.hpp"
#include <glm/ext/matrix_clip_space.hpp>

void initWindow();
void endWindow();
void initLettuce();
void endLettuce();
void mainLoop();
void draw();
void buildCmds();
void recordCmds();
void genTorus();
void updateData();

using namespace Lettuce::Core;

GLFWwindow *window;
const int width = 800;
const int height = 600;
Instance instance;
Device device;
Swapchain swapchain;
Semaphore renderFinished;
Buffer vertexBuffer;
Buffer indexBuffer;
DescriptorLayout descriptorLayout;
Descriptor descriptor;
PipelineConnector connector;
GraphicsPipeline pipeline;
Compilers::GLSLCompiler compiler;
Shader fragmentShader;
Shader vertexShader;

struct Vertex
{
    glm::vec3 position;
};
struct DataUBO
{
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
} dataUBO;
struct DataPush
{
    glm::vec3 color;
} dataPush;
std::vector<Vertex> vertices;
std::vector<uint32_t> indices;

const std::string fragmentShaderText = R"(layout (push_constant, std430) uniform pushData {
    vec3 color;
};
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(color, 1.0);
})";
const std::string vertexShaderText = R"(layout (binding = 0, location = 0) in vec3 pos;
layout (binding = 0, location = 0) uniform DataUBO {
    mat4 projection;
    mat4 model;
    mat4 view;
} ubo;

void main()
{
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(pos,1.0); 
})";

VkCommandPool pool;
VkCommandBuffer cmd;

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
    instance.Create("DonutSample", Lettuce::Core::Version{0, 1, 0, 0}, {});
    instance.CreateSurface(glfwGetWin32Window(window), GetModuleHandle(nullptr));
    auto gpus = instance.getGPUs();
    for (auto gpu : gpus)
    {
        std::cout << "available device: " << gpu.deviceName << std::endl;
        std::cout << "    graphics family: " << gpu.graphicsFamily.value() << std::endl;
        std::cout << "    present family : " << gpu.presentFamily.value() << std::endl;
    }
    // create device
    Features features;
    features.MeshShading = true;
    features.ConditionalRendering = true;
    features.MemoryBudget = true;
    device.Create(instance, gpus.front(), features);
    swapchain.Create(device, width, height);

    // create sync objects
    renderFinished.Create(device, 0);

    // create rendering resources
    genTorus();
    vertexBuffer = Buffer::CreateVertexBuffer(device, vertices);
    indexBuffer = Buffer::CreateIndexBuffer(device, indices);

    descriptorLayout.AddDescriptorBinding(0, DescriptorType::UniformBuffer, PipelineStage::Vertex);
    descriptorLayout.Build(device, {1});
    descriptor = descriptorLayout.GetDescriptors().front();
    descriptor.AddUpdateInfo<DataUBO>(0, vertexBuffer);
    descriptor.Update();

    connector.AddDescriptor(descriptorLayout);
    connector.AddPushConstant<DataPush>(0, PipelineStage::Fragment);
    connector.Build(device);

    // add pipeline stuff here
    vertexShader.Create(device, compiler, vertexShaderText, "main", "vertex.glsl", PipelineStage::Vertex, true);
    fragmentShader.Create(device, compiler, fragmentShaderText, "main", "fragment.glsl", PipelineStage::Fragment, true);

    pipeline.AddVertexBindingDescription<Vertex>(0);                     // binding = 0
    pipeline.AddVertexAttribute(0, 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT); // layout(location = 0) in vec3 pos;
    pipeline.AddShaderStage(vertexShader);
    pipeline.AddShaderStage(fragmentShader);
    pipeline.Build(device, connector, swapchain);

    vertexShader.Destroy();
    fragmentShader.Destroy();

    buildCmds();
}

void buildCmds()
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

void updateData()
{
    dataPush.color = {0.5f, 0.4f, 0.3f};
    dataUBO.projection = glm::perspective(glm::radians(45.0f), (float)width/(float)height, 0.1f, 100.0f);
    dataUBO.model = glm::mat4();
    dataUBO.view = glm::mat4(1.0f);
    
}

void recordCmds()
{
    // rendering cmd

    VkImageSubresourceRange imgSubresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    checkResult(vkResetCommandBuffer(cmd, 0));
    //    VkClearValue clearValues[2];
    //    clearValues[0].color = {{0.5f, 0.5f, 0.5f, 1.0f}};
    //    clearValues[1].depthStencil = {1, 0};
    VkClearValue clearValue;
    clearValue.color = {{0.5f, 0.5f, 0.5f, 1.0f}};

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
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline._pipeline);
    VkDeviceSize size = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &(vertexBuffer._buffer), &size);
    vkCmdBindIndexBuffer(cmd, indexBuffer._buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, connector._pipelineLayout, 0, 1, &(descriptor._descriptorSet), 0, nullptr);

    vkCmdPushConstants(cmd, connector._pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DataPush), &dataPush);
    vkCmdSetLineWidth(cmd, 1.0f);
    VkViewport viewport = {0, 0, width, height, 0.0f, 1.0f};
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {width, height}};
    vkCmdSetScissor(cmd, 0, 1, &scissor);
    vkCmdSetPrimitiveTopology(cmd, VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    vkCmdDrawIndexed(cmd, indices.size(), 1, 0, 0, 0);
    vkCmdEndRenderPass(cmd);

    checkResult(vkEndCommandBuffer(cmd));
}

uint64_t renderFinishedValue = 0;

/*
|               |        waits for nothing
|               |        signals semaphore to r+1

waits for r+1
r ++
*/
void draw()
{
    swapchain.AcquireNextImage();

    recordCmds();

    // sends rendering cmd

    VkCommandBufferSubmitInfo cmdSI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = cmd,
        .deviceMask = 0,
    };

    VkSemaphoreSubmitInfo signalSI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = renderFinished._semaphore,
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
    renderFinished.Wait(renderFinishedValue + 1);

    swapchain.Present();

    renderFinishedValue++;
}

void endLettuce()
{
    vkFreeCommandBuffers(device._device, pool, 1, &cmd);
    vkDestroyCommandPool(device._device, pool, nullptr);
    pipeline.Destroy();
    connector.Destroy();
    descriptorLayout.Destroy();
    vertexBuffer.Destroy();
    indexBuffer.Destroy();
    renderFinished.Destroy();
    swapchain.Destroy();
    device.Destroy();
    instance.Destroy();
}

void mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        std::cout << "-------------- frame ---------------" << std::endl;
        glfwPollEvents();
        draw();
    }
    device.Wait();
}

void initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(width, height, "Lettuce DonutSample", nullptr, nullptr);
}

void endWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void genTorus()
{
    float radiusMajor = 10;
    float radiusMinor = 5;
    // float sectorStep = 10;
    float sectorCount = 10;
    // float sideStep;
    float sideCount = 10;

    float theta;
    float phi;

    const float pi = std::numbers::pi_v<float>;

    // add vertices
    for (int i = 0; i <= sideCount; i++)
    {
        phi = pi - (2 * pi * (i / sideCount));
        for (int j = 0; j <= sectorCount; j++)
        {
            theta = (2 * pi * (j / sectorCount));
            float x = (radiusMajor + (radiusMinor * std::cos(phi))) * (std::cos(theta));
            float y = (radiusMajor + (radiusMinor * std::cos(phi))) * (std::sin(theta));
            float z = (radiusMinor * std::sin(phi));
            vertices.push_back({{x, y, z}});
        }
    }
    // add indices

    // indices
    //  k1--k1+1
    //  |  / |
    //  | /  |
    //  k2--k2+1
    unsigned int k1, k2;
    for (int i = 0; i < sideCount; ++i)
    {
        k1 = i * (sectorCount + 1); // beginning of current side
        k2 = k1 + sectorCount + 1;  // beginning of next side

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector
            indices.push_back(k1);
            indices.push_back(k2);
            indices.push_back(k1 + 1); // k1---k2---k1+1
            indices.push_back(k1 + 1);
            indices.push_back(k2);
            indices.push_back(k2 + 1); // k1+1---k2---k2+1
        }
    }
}