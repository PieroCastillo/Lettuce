//
// Created by piero on 17/11/2024.
//
#include "LettuceSampleApp.hpp"
#include "Lettuce/Lettuce.X3D.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include <iostream>
#include <vector>
#include <cmath>
#include <numbers>
#include <tuple>
#include <memory>

using namespace Lettuce::Core;
using namespace Lettuce::X3D;

class GeometrySample : public LettuceSampleApp
{
public:
    Lettuce::Core::Compilers::GLSLCompiler compiler;
    std::shared_ptr<Lettuce::Core::RenderPass> renderpass;
    /* sync objects*/
    std::shared_ptr<Lettuce::Core::Semaphore> renderFinished;
    /* rendering objects */


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

    VkCommandPool pool;
    VkCommandBuffer cmd;

    void createRenderPass()
    {
        renderpass = std::make_shared<RenderPass>(device);
        renderpass->AddAttachment(0, AttachmentType::Color,
                                  swapchain->imageFormat,
                                  LoadOp::Clear,
                                  StoreOp::Store,
                                  LoadOp::DontCare,
                                  StoreOp::DontCare,
                                  ImageLayout::Undefined,
                                  ImageLayout::PresentSrc,
                                  ImageLayout::ColorAttachmentOptimal);
        renderpass->AddSubpass(0, BindPoint::Graphics, {0});
        renderpass->AddDependency(VK_SUBPASS_EXTERNAL, 0,
                                  AccessStage::ColorAttachmentOutput,
                                  AccessStage::ColorAttachmentOutput,
                                  AccessBehavior::None,
                                  AccessBehavior::ColorAttachmentWrite);

        renderpass->AddDependency(0, VK_SUBPASS_EXTERNAL,
                                  AccessStage::ColorAttachmentOutput,
                                  AccessStage::ColorAttachmentOutput,
                                  AccessBehavior::ColorAttachmentWrite,
                                  AccessBehavior::None);
        renderpass->Assemble();
        for (auto &view : swapchain->swapChainImageViews)
        {
            renderpass->AddFramebuffer(width, height, {view});
        }
        renderpass->BuildFramebuffers();
    }

    void onResize()
    {
        renderpass->DestroyFramebuffers();
        for (auto &view : swapchain->swapChainImageViews)
        {
            renderpass->AddFramebuffer(width, height, {view});
        }
        renderpass->BuildFramebuffers();
    }

    void createObjects()
    {
        renderFinished = std::make_shared<Lettuce::Core::Semaphore>(device, 0);
        buildCmds();
        genScene();

        camera = Lettuce::X3D::Camera3D::Camera3D(width, height);
        beforeResize();
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
            .level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        checkResult(vkAllocateCommandBuffers(device->_device, &cmdAI, &cmd));
    }

    void updateData()
    {
        dataPush.color = glm::vec3(1.0f, 0.5f, 0.31f);
        // camera.SetPosition(glm::vec3(20, 20, 30));
        camera.Update();
        dataUBO.projectionView = camera.GetProjectionView();
        dataUBO.model = glm::mat4(1.0f);
        dataUBO.cameraPos = camera.eye;
        // dataUBO.cameraPos = glm::vec3(30);

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
            .image = swapchain->swapChainImages[(int)swapchain->index],
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        };

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

        VkRenderPassBeginInfo renderPassBI = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = renderpass->_renderPass,
            .framebuffer = renderpass->_framebuffers[(int)swapchain->index],
            .renderArea = renderArea,
            .clearValueCount = 1,
            .pClearValues = &clearValue,
        };

        vkCmdBeginRenderPass(cmd, &renderPassBI, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

        
       
        vkCmdEndRenderPass(cmd);

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
            // .waitSemaphoreInfoCount = 1,
            // .pWaitSemaphoreInfos = &waitSI,
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

    void beforeResize()
    {
        camera = Lettuce::X3D::Camera3D::Camera3D(width, height);
        camera.Reset(glm::vec3(20, 20, 30), glm::vec3(0.0f), glm::vec3(0.57735026919)); // 1 / sqrt(3)
    }

    void destroyObjects()
    {
        vkFreeCommandBuffers(device->_device, pool, 1, &cmd);
        vkDestroyCommandPool(device->_device, pool, nullptr);

       
        renderFinished->Release();
        renderpass->DestroyFramebuffers();
        renderpass->Release();
    }

    void genScene()
    {
        /*geometries*/
        std::vector<Geometries::Sphere> spheres;
    }

};

int main()
{
    GeometrySample app;
    app.appName = "Geometry Sample";
    app.title = "Geometry Sample";
    app.features = {
        .FragmentShadingRate = false,
        .PresentWait = false,
        .ExecutionGraphs = false,
        .MeshShading = false,
        .RayTracing = false,
        .Video = false,
        .MemoryBudget = false,
        .ConditionalRendering = false,
    };
    app.run();
    return 0;
}