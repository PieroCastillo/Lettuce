//
// Created by piero on 10/02/2024.
//
#include <iostream>
#include <vector>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
// #define VK_USE_PLATFORM_WIN32_KHR
// #include <volk.h>

#include "Lettuce/Lettuce.Core.hpp"

void initWindow();
void endWindow();
void initLettuce();
void endLettuce();
void mainLoop();
void draw();
void buildCmds();
void recordCmds();

using namespace Lettuce::Core;

GLFWwindow *window;
const int width = 800;
const int height = 600;
Lettuce::Core::Instance instance;
Lettuce::Core::Device device;
Lettuce::Core::Swapchain swapchain;
Lettuce::Core::TSemaphore renderFinished;

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
    instance.Create("ClearScreenSample", Lettuce::Core::Version{0, 1, 0, 0}, {});
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
    swapchain.Create(device, width, height);
    renderFinished.Create(device, 0);
    acquire.Create(device);
    buildCmds();
}

void buildCmds()
{
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

void recordCmds()
{
    VkClearValue clearValues[2];
    VkClearValue color, depth;
    clearValues[0].color = {{0.5f, 0.5f, 0.5f, 1.0f}};
    clearValues[1].depthStencil = {1, 0};

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
        .clearValueCount = 2,
        .pClearValues = clearValues,
    };

    vkCmdBeginRenderPass(cmd, &renderPassBI, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(cmd);

    checkResult(vkEndCommandBuffer(cmd));
}

uint64_t renderFinishedValue = 0;
void draw()
{
    swapchain.AcquireNextImage();

    recordCmds();

    VkPipelineStageFlags waitMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // VkSubmitInfo submitI = {
    //     .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    //     .waitSemaphoreCount = 1,
    //     .pWaitSemaphores = &acquire._semaphore,
    //     .pWaitDstStageMask = &waitMask,
    //     .commandBufferCount = 1,
    //     .pCommandBuffers = &cmd,
    //     .signalSemaphoreCount = 1,
    //     .pSignalSemaphores = &render._semaphore,
    // };

    VkCommandBufferSubmitInfo cmdSI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = cmd,
        .deviceMask = 0,
    };

    // VkSemaphoreSubmitInfo waitSI = {
    //     .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
    //     .semaphore = renderFinished._semaphore,
    //     .deviceIndex = 0,
    // };

    VkSemaphoreSubmitInfo signalSI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = renderFinished._semaphore,
        .value = renderFinishedValue + 1,
        .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .deviceIndex = 0,
    };

    VkSubmitInfo2 submit2I = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        //.waitSemaphoreInfoCount = 1,
        // .pWaitSemaphoreInfos = &waitSI,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &cmdSI,
        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos = &signalSI,
    };

    checkResult(vkQueueSubmit2(device._graphicsQueues[0], 1, &submit2I, VK_NULL_HANDLE));
    // checkResult(vkQueueSubmit(device._graphicsQueues[0], 1, &submitI, VK_NULL_HANDLE));
    swapchain.Present();
    renderFinished.Wait(renderFinishedValue+1);
    renderFinishedValue++;
}

void endLettuce()
{
    vkFreeCommandBuffers(device._device, pool, 1, &cmd);
    vkDestroyCommandPool(device._device, pool, nullptr);
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
    window = glfwCreateWindow(width, height, "Lettuce ClearScreenSample", nullptr, nullptr);
}

void endWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}