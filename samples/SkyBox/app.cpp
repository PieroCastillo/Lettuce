//
// Created by piero on 17/11/2024.
//
#include <iostream>
#include <vector>
#include <numbers>
#include <tuple>
#include <memory>
#include <string>

#include "LettuceSampleApp.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace Lettuce::Core;

class SkyBoxSample : public LettuceSampleApp
{
public:
    Lettuce::Core::Compilers::GLSLCompiler compiler;
    /* sync objects*/
    std::shared_ptr<Lettuce::Core::Semaphore> renderFinished;

    VkCommandPool pool;
    VkCommandBuffer cmd;

    struct DataPush
    {
    };

    void createObjects()
    {
        renderFinished = std::make_shared<Lettuce::Core::Semaphore>(device, 0);
        releaseQueue.Push(renderFinished);

        buildCmds();

        beforeResize();
    }

    void loadCubeMap()
    {
        // we need:
        // shaders
        // textures
        // geometry

        // cube geometry data
        Geometries::Prism cube(glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
        auto cubeBuffer = std::make_shared<BufferResource>(device, cube.GetFullSize(), VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_2_INDEX_BUFFER_BIT);
        auto cubePool = std::make_shared<ResourcePool>(device, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        cubePool->AddResource(cubeBuffer);
        cubePool->Bind();

        char *cubeBufferData = (char *)malloc(cube.GetFullSize());
        memcpy((void *)cubeBuffer, (void *)cube.points.data(), cube.info.vertBlock.size);
        memcpy((void *)(cubeBuffer + cube.info.vertBlock.size), (void *)cube.indices.data(), cube.info.indexBlock.size);

        cubePool->Map();
        cubePool->SetData((void *)cubeBufferData, 0, cube.GetFullSize());
        cubePool->UnMap();
        free(cubeBufferData);

        // textures
        std::vector<std::string> texturePaths = {
            "assets/back.jpg",
            "assets/bottom.jpg",
            "assets/front.jpg",
            "assets/left.jpg",
            "assets/right.jpg",
            "assets/top.jpg",
        };

        auto cubeTexture = std::make_shared<ImageResource>(device, 0, 0, 1,
                                                           VK_IMAGE_TYPE_2D,
                                                           VK_IMAGE_USAGE_SAMPLED_BIT, 1, 6,
                                                           VK_FORMAT_R32G32B32_SFLOAT,
                                                           VK_IMAGE_LAYOUT_PREINITIALIZED);

        std::vector<std::shared_ptr<BufferResource>> imgs_temp;
        std::vector<std::pair<void *, uint32_t>> imgPtrs;
        for (auto &path : texturePaths)
        {
            int texWidth, texHeight, texChannels;
            if (!std::filesystem::exists(path))
            {
                // std::cout << " file " << filenames[i] << " does not exist" << std::endl;
                std::abort();
            }
            stbi_uc *pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

            if (!pixels)
            {
                std::abort();
            }

            imgPtrs.push_back({(void *)pixels, 4 * texHeight * texWidth});

            auto img_temp = std::make_shared<BufferResource>(device, 4 * texHeight * texWidth, VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT);

            imgs_temp.push_back(img_temp);
            // images_bindable.push_back(img_dst);
            pool_temp->AddResource(img_temp);
            // pool_images->AddResource(img_dst);
        }

        // descriptors and other stuff
        auto descriptors = std::make_shared<Descriptors>(device);
        descriptors->AddBinding(0, 1, DescriptorType::CombinedImageSampler, PipelineStage::Fragment, 1);
        descriptors->Assemble();

        auto sampler = std::make_shared<Sampler>(device);

        auto pipelineLayout = std::make_shared<PipelineLayout>(device, descriptors);
        pipelineLayout->AddPushConstant<DataPush>(VK_SHADER_STAGE_FRAGMENT_BIT);
        pipelineLayout->Assemble();
    }

    void buildCmds()
    {
        // rendering
        VkCommandPoolCreateInfo poolCI = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = device->_gpu.graphicsFamily.value(),
        };
        checkResult(vkCreateCommandPool(device->GetHandle(), &poolCI, nullptr, &pool));

        VkCommandBufferAllocateInfo cmdAI = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        checkResult(vkAllocateCommandBuffers(device->GetHandle(), &cmdAI, &cmd));
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
            .semaphore = renderFinished->GetHandle(),
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
        vkFreeCommandBuffers(device->GetHandle(), pool, 1, &cmd);
        vkDestroyCommandPool(device->GetHandle(), pool, nullptr);
    }
};

int main()
{
    SkyBoxSample app;
    app.appName = "Sky Box Sample";
    app.title = "Sky Box Sample";
    app.run();
    return 0;
}