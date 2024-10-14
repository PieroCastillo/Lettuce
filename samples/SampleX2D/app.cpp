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
#include "Lettuce/Lettuce.X2D.hpp"
#include <glm/ext/matrix_clip_space.hpp>

void initWindow();
void endWindow();
void initLettuce();
void endLettuce();
void mainLoop();
void draw();
void buildCmds();
void recordCmds();
void updateData();
std::tuple<uint32_t, uint32_t> resizeCall();

using namespace Lettuce::Core;
using namespace Lettuce::X2D;

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

//Camera2D camera;


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
    //camera = Camera2D(width, height);

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
    instance.Create("Sample X2D", Lettuce::Core::Version{0, 1, 0, 0}, {});
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
    swapchain.Create(device, width, height);

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

    //camera = Camera3D(width, height);
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
    window = glfwCreateWindow(width, height, "Lettuce X2D Sample", nullptr, nullptr);
}

void endWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}