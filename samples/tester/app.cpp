//
// Created by piero on 08/02/2024.
//
#include <iostream>
#include <vector>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <numbers>
#include <tuple>
#include <memory>

#include "Lettuce/Lettuce.Core.hpp"
#include "Lettuce/Lettuce.X3D.hpp"
#include <glm/ext/matrix_clip_space.hpp>

void initWindow();
void endWindow();
void initLettuce();
void endLettuce();
void mainLoop();
void draw();
void buildCmds();
void recordCmds();
void genRect();
void updateData();
std::tuple<uint32_t, uint32_t> resizeCall();

using namespace Lettuce::Core;
using namespace Lettuce::X3D;

GLFWwindow *window;
int width = 800;
int height = 600;
/*Basic stuff for Lettuce usage*/
Instance instance;
Device device;
Swapchain swapchain;
RenderPass renderpass;
/* sync objects*/
Semaphore renderFinished;
/* rendering object */
Buffer vertexBuffer;
Buffer indexBuffer;
Buffer uniformBuffer;
Buffer lightBuffer;
Descriptors descriptor;
PipelineLayout connector;
GraphicsPipeline pipeline;
Compilers::GLSLCompiler compiler;
Shader fragmentShader;
Shader vertexShader;

Camera3D camera;

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
};
struct DataUBO
{
    glm::mat4 projectionView;
    glm::mat4 model;
    glm::vec3 cameraPos;
} dataUBO;
struct DataPush
{
    glm::vec3 color;
} dataPush;
std::vector<Vertex> vertices;
std::vector<uint32_t> indices;

const std::string fragmentShaderText = R"(#version 450
#
layout (push_constant) uniform pushData {
    vec3 color;
};
layout (location = 0) in vec3 norm;
layout (location = 0) out vec4 outColor;
void main()
{
    outColor = vec4(normalize(norm),1.0);
})";

const std::string vertexShaderText = R"(#version 450
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 0) out vec3 norm;

layout (set = 0, binding = 0) uniform DataUBO {
    mat4 projectionView;
    mat4 model;
    vec3 cameraPos;
} ubo;

void main()
{   
    norm = mat3(transpose(inverse(ubo.model))) * normal;
    gl_Position = ubo.projectionView * ubo.model * vec4(pos,1.0);
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

std::tuple<uint32_t, uint32_t> resizeCall()
{
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    width = w;
    height = h;
    camera = Camera3D(width, height);

    return std::make_tuple((uint32_t)w, (uint32_t)h);
}

void onResize()
{
    renderpass.DestroyFramebuffers();
    for (auto &view : swapchain.swapchainTextureViews)
    {
        renderpass.AddFramebuffer(width, height, {view});
    }
    renderpass.BuildFramebuffers();
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
    }
    // create device
    Features features;
    features.MeshShading = false;
    features.ConditionalRendering = false;
    features.MemoryBudget = false;
    device.Create(instance, gpus.front(), features);
    std::cout << "-------- device created ----------" << std::endl;
    swapchain.Create(device, width, height);
    std::cout << "-------- swapchain created ----------" << std::endl;

    renderpass.AddAttachment(0, AttachmentType::Color,
                             swapchain.imageFormat,
                             LoadOp::Clear,
                             StoreOp::Store,
                             LoadOp::DontCare,
                             StoreOp::DontCare,
                             ImageLayout::Undefined,
                             ImageLayout::PresentSrc,
                             ImageLayout::ColorAttachmentOptimal);
    renderpass.AddSubpass(0, BindPoint::Graphics, {0});
    renderpass.AddDependency(VK_SUBPASS_EXTERNAL, 0,
                             AccessStage::ColorAttachmentOutput,
                             AccessStage::ColorAttachmentOutput,
                             AccessBehavior::None,
                             AccessBehavior::ColorAttachmentWrite);

    renderpass.AddDependency(0, VK_SUBPASS_EXTERNAL,
                             AccessStage::ColorAttachmentOutput,
                             AccessStage::ColorAttachmentOutput,
                             AccessBehavior::ColorAttachmentWrite,
                             AccessBehavior::None);
    renderpass.Build(device);
    std::cout << "-------- renderpass created ----------" << std::endl;
    for (auto &view : swapchain.swapchainTextureViews)
    {
        renderpass.AddFramebuffer(width, height, {view});
    }
    renderpass.BuildFramebuffers();
    std::cout << "-------- framebuffers created ----------" << std::endl;

    swapchain.SetResizeFunc(resizeCall, onResize);

    // create sync objects
    renderFinished.Create(device, 0);
    // build command buffers
    buildCmds();

    // create rendering resources
    genRect();
    std::cout << "-------- torus created ----------" << std::endl;
    vertexBuffer = Buffer::CreateVertexBuffer(device, vertices);
    std::cout << "-------- vertex buffer created ----------" << std::endl;
    indexBuffer = Buffer::CreateIndexBuffer(device, indices);
    std::cout << "-------- index buffer created ----------" << std::endl;

    uniformBuffer = Buffer::CreateUniformBuffer<DataUBO>(device);
    uniformBuffer.Map();
    uniformBuffer.SetData(&dataUBO);
    std::cout << "-------- uniform buffer created ----------" << std::endl;

    descriptor.AddBinding(0, 0, DescriptorType::UniformBuffer, PipelineStage::Vertex, 1);
    descriptor.Build(device);
    std::cout << "-------- descriptor created ----------" << std::endl;
    descriptor.AddUpdateInfo<DataUBO>(0, 0, {uniformBuffer});
    descriptor.Update();
    std::cout << "-------- descriptor updated ----------" << std::endl;

    connector.AddPushConstant<DataPush>(0, PipelineStage::Fragment);
    connector.Build(device, descriptor);

    // add pipeline stuff here
    vertexShader.Create(device, compiler, vertexShaderText, "main", "vertex.glsl", PipelineStage::Vertex, true);
    fragmentShader.Create(device, compiler, fragmentShaderText, "main", "fragment.glsl", PipelineStage::Fragment, true);

    pipeline.AddVertexBindingDescription<Vertex>(0);                                  // binding = 0
    pipeline.AddVertexAttribute(0, 0, 0, VK_FORMAT_R32G32B32_SFLOAT);                 // layout(location = 0) in vec3 pos;
    pipeline.AddVertexAttribute(0, 1, sizeof(glm::vec3), VK_FORMAT_R32G32B32_SFLOAT); // layout(location = 1) in vec3 normal;
    pipeline.AddShaderStage(vertexShader);
    pipeline.AddShaderStage(fragmentShader);
    pipeline.Build(device, connector, renderpass, 0,
                   {.rasterization = {
                        .frontFace = VK_FRONT_FACE_CLOCKWISE,
                    },
                    .colorBlend = {
                        .attachments = {
                            {
                                .colorWriteMask = VK_COMPONENT_SWIZZLE_R | VK_COMPONENT_SWIZZLE_G | VK_COMPONENT_SWIZZLE_B | VK_COMPONENT_SWIZZLE_A,
                            },
                        },
                    }});
    std::cout << "-------- pipeline created ----------" << std::endl;

    vertexShader.Destroy();
    fragmentShader.Destroy();

    camera = Camera3D(width, height);
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
    dataPush.color = glm::vec3(1.0f, 0.5f, 0.31f);
    camera.SetPosition(glm::vec3(20, 20, 30));
    // camera.SetPosition(glm::vec3(30));
    camera.SetCenter(glm::vec3(0.0f, 0.0f, 0.0f));
    camera.Update();
    dataUBO.projectionView = camera.GetProjectionView();
    dataUBO.model = glm::mat4(1.0f);
    dataUBO.cameraPos = glm::vec3(20, 20, 30);
    // dataUBO.cameraPos = glm::vec3(30);

    uniformBuffer.SetData(&dataUBO);
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

    VkImageMemoryBarrier2 imageBarrier2 = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = swapchain.swapChainImages[(int)swapchain.index],
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };

    VkDependencyInfo dependencyI = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &imageBarrier2,
    };

    vkCmdPipelineBarrier2(cmd, &dependencyI);

    VkRect2D renderArea;
    renderArea.extent.height = swapchain.height;
    renderArea.extent.width = swapchain.width;
    renderArea.offset.x = 0;
    renderArea.offset.y = 0;

    VkRenderPassBeginInfo renderPassBI = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderpass._renderPass,
        .framebuffer = renderpass._framebuffers[(int)swapchain.index],
        .renderArea = renderArea,
        .clearValueCount = 1,
        .pClearValues = &clearValue,
    };

    vkCmdBeginRenderPass(cmd, &renderPassBI, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline._pipeline);
    VkDeviceSize size = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &(vertexBuffer._buffer), &size);
    vkCmdBindIndexBuffer(cmd, indexBuffer._buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, connector._pipelineLayout, 0, 1, descriptor._descriptorSets.data(), 0, nullptr);

    vkCmdPushConstants(cmd, connector._pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DataPush), &dataPush);
    vkCmdSetLineWidth(cmd, 1.0f);
    VkViewport viewport = {0, 0, (float)width, (float)height, 0.0f, 1.0f};
    // vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetViewportWithCount(cmd, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {(float)width, (float)height}};
    // vkCmdSetScissor(cmd, 0, 1, &scissor);
    vkCmdSetScissorWithCount(cmd, 1, &scissor);
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
    descriptor.Destroy();

    uniformBuffer.Unmap();
    uniformBuffer.Destroy();
    vertexBuffer.Destroy();
    indexBuffer.Destroy();
    renderFinished.Destroy();
    renderpass.DestroyFramebuffers();
    renderpass.Destroy();
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
        updateData();
        draw();
    }
    device.Wait();
}

void initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(width, height, "Lettuce DonutSample", nullptr, nullptr);
}

void endWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void genRect()
{
    for(auto vec : std::vector<glm::vec2>{{-0.5,0.5},{0.5,0.5},{0.5,-0.5},{-0.5,-0.5} })//v0,v1,v2,v3
    {
        vec = glm::mat2(20,0,30,0)*vec;
        auto norm = glm::normalize(vec);
        vertices.push_back({{vec.x, vec.y, 0.5f},{norm.x, norm.y, 0}});
    }
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(2);
    indices.push_back(3);
    indices.push_back(0);
}