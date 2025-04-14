//
// Created by piero on 7/04/2025.
//
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <chrono>

#include <taskflow/taskflow.hpp>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "Lettuce/Lettuce.Core.hpp"
#include "Lettuce/Lettuce.Foundation.hpp"
#include "Lettuce/Lettuce.X3D.hpp"

#include "Core.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Renderer::Core;

class App : public AppBase
{
    View mainView;

public:
    Lettuce::Core::Compilers::GLSLCompiler compiler;
    /* sync objects*/
    std::shared_ptr<Lettuce::Core::Semaphore> renderFinished;

    VkCommandPool pool;
    VkCommandBuffer cmd;

    void createObjects()
    {
        renderFinished = std::make_shared<Lettuce::Core::Semaphore>(device, 0);
        releaseQueue.Push(renderFinished);
        beforeResize();
    }

    void genScene()
    {
    }

    void createGeometry()
    {
    }

    void createPipelines()
    {
    }

    void createDescriptors()
    {
    }

    void buildCmds()
    {
        // rendering
        VkCommandPoolCreateInfo poolCI = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = device->_gpu.graphicsFamily.value(),
        };
        checkResult(vkCreateCommandPool(device->_device, &poolCI, nullptr, &pool));

        VkCommandBufferAllocateInfo cmdAI = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        checkResult(vkAllocateCommandBuffers(device->_device, &cmdAI, &cmd));
    }

    void recordCmds()
    {
        // rendering cmd

        VkImageSubresourceRange imgSubresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

        checkResult(vkResetCommandBuffer(cmd, 0));

        VkCommandBufferBeginInfo cmdBI = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
        };
        checkResult(vkBeginCommandBuffer(cmd, &cmdBI));

        VkImageMemoryBarrier2 imageBarrier2 = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = swapchain->swapChainImages[(int)swapchain->index],
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        };

        // VkMemoryBarrier2 memBarriers[] = {memBarrier1, memBarrier2};

        VkDependencyInfo dependencyI = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &imageBarrier2,
        };

        vkCmdPipelineBarrier2(cmd, &dependencyI);

        VkRect2D renderArea;
        renderArea.extent.height = swapchain->height;
        renderArea.extent.width = swapchain->width;
        renderArea.offset.x = 0;
        renderArea.offset.y = 0;

        VkClearValue clearValue;
        clearValue.color = {{0.5f, 0.5f, 0.5f, 1.0f}};

        VkRenderingAttachmentInfo colorAttachment = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = swapchain->swapChainImageViews[(int)swapchain->index],
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = clearValue,
        };

        VkRenderingInfo renderingInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = renderArea,
            .layerCount = 1,
            .viewMask = 0,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachment,
        };

        vkCmdBeginRendering(cmd, &renderingInfo);

        vkCmdEndRendering(cmd);

        imageBarrier2.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        imageBarrier2.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        imageBarrier2.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        imageBarrier2.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
        imageBarrier2.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageBarrier2.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        dependencyI.bufferMemoryBarrierCount = 0;
        dependencyI.pBufferMemoryBarriers = 0;
        vkCmdPipelineBarrier2(cmd, &dependencyI);

        checkResult(vkEndCommandBuffer(cmd));
    }

    uint64_t renderFinishedValue = 0;
    void draw()
    {
        swapchain->AcquireNextImage();

        recordCmds();

        // sends rendering cmd

        VkCommandBufferSubmitInfo cmdSI = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = cmd,
            .deviceMask = 0,
        };

        VkSemaphoreSubmitInfo signalSI = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = renderFinished->_semaphore,
            .value = renderFinishedValue + 1,
            .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .deviceIndex = 0,
        };

        VkSubmitInfo2 submit2I = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .commandBufferInfoCount = 1,
            .pCommandBufferInfos = &cmdSI,
            .signalSemaphoreInfoCount = 1,
            .pSignalSemaphoreInfos = &signalSI,
        };

        checkResult(vkQueueSubmit2(device->_graphicsQueues[0], 1, &submit2I, VK_NULL_HANDLE));
        renderFinished->Wait(renderFinishedValue + 1);

        swapchain->Present();

        renderFinishedValue++;
    }

    void destroyObjects()
    {
        vkFreeCommandBuffers(device->_device, pool, 1, &cmd);
        vkDestroyCommandPool(device->_device, pool, nullptr);
    }
};

int main()
{
    std::cout << "Hello World! " << std::endl;

    App app;
    app.appName = "Lettuce Renderer";
    app.title = "Lettuce Renderer";

    tf::Executor executor;
    tf::Taskflow taskflow;

    auto [w, l, o, cmds, g, p, d, loop, dO, dL, ew] = taskflow.emplace(
        [&]()
        { app.initWindow(); },
        [&]()
        { app.initializeLettuce(); },
        [&]()
        { app.createObjects(); },
        [&]()
        { app.buildCmds(); },
        [&]()
        { app.createGeometry(); },
        [&]()
        { app.createPipelines(); },
        [&]()
        { app.createDescriptors(); },
        [&]()
        { app.mainLoop(); },
        [&]()
        { app.destroyObjects(); },
        [&]()
        { app.destroyLettuce(); },
        [&]()
        { app.endWindow(); });

    w.precede(l);
    l.precede(o, cmds, g, p, d);
    loop.succeed(o, cmds, g, p, d);
    loop.precede(dO);
    dO.precede(dL);
    dL.precede(ew);

    executor.run(taskflow).wait();

    return 0;
}