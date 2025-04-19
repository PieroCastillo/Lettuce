//
// Created by piero on 4/04/2024.
//
#include "LettuceSampleApp.hpp"
#include <iostream>
#include <vector>
#include <numbers>
#include <tuple>
#include <memory>
#include <string>

#include "Lettuce/Lettuce.X3D.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::X3D;

class LoadModelSample : public LettuceSampleApp
{
public:
    Lettuce::Core::Compilers::GLSLCompiler compiler;
    /* sync objects*/
    std::shared_ptr<Lettuce::Core::Semaphore> renderFinished;

    std::shared_ptr<Lettuce::Core::Descriptors> descriptor;
    std::shared_ptr<Lettuce::Core::PipelineLayout> layout;
    std::shared_ptr<Lettuce::Core::GraphicsPipeline> pipeline;

    std::shared_ptr<Lettuce::Core::ShaderModule> fragmentShader;
    std::shared_ptr<Lettuce::Core::ShaderModule> vertexShader;

    std::shared_ptr<Scene> scene;

    VkCommandPool pool;
    VkCommandBuffer cmd;

    struct DataPush
    {
        glm::mat4 projectionView;
        glm::mat4 model;
        glm::vec3 cameraPos;
    };

    std::string vertexShaderText = R"(#version 450
    layout (location = 0) in vec3 pos;
    layout (location = 1) in vec3 normal;
    layout (location = 2) in vec4 tangent;
    // layout (location = 0) out vec3 norm;
    
    layout (push_constant) uniform DataPush {
        mat4 projectionView;
        mat4 model;
        vec3 cameraPos;
    } data;
    
    void main()
    {   
        // vec3 norm = mat3(transpose(inverse(data.model))) * normal;
        gl_Position = data.projectionView * data.model * vec4(pos,1.0);
    })";

    std::string fragmentShaderText = R"(#version 450
    // layout (push_constant) uniform pushData {
    //     vec3 color;
    // } data;
    // layout (location = 0) in vec3 norm;

    layout (location = 0) out vec4 outColor;
    
    void main()
    {
        outColor = vec4(1.0);
    })";

    void loadScene()
    {
        auto gltfFile = "assets/Fox.gtlf";
        scene->LoadFromFile(device, gltfFile);
    }

    void createObjects()
    {
        renderFinished = std::make_shared<Lettuce::Core::Semaphore>(device, 0);
        releaseQueue.Push(renderFinished);

        std::cout << "scene is not loaded yet" << std::endl;
        loadScene();
        std::cout << "scene loaded" << std::endl;

        layout = std::make_shared<PipelineLayout>(device);
        layout->AddPushConstant<DataPush>(VK_SHADER_STAGE_VERTEX_BIT); // pass transform to vertex stage
        layout->Assemble();

        std::cout << "layout built" << std::endl;

        fragmentShader = std::make_shared<ShaderModule>(device, compiler, fragmentShaderText, "main", "frag.glsl", PipelineStage::Fragment, true);
        vertexShader = std::make_shared<ShaderModule>(device, compiler, vertexShaderText, "main", "vert.glsl", PipelineStage::Vertex, true);

        std::cout << "shaders built" << std::endl;

        pipeline = std::make_shared<GraphicsPipeline>(device, layout);
        pipeline->AddVertexBindingDescription<Vertex>(0);                                     // binding = 0
        pipeline->AddVertexAttribute(0, 0, 0, VK_FORMAT_R32G32B32_SFLOAT);                    // layout(location = 0) in vec3 pos;
        pipeline->AddVertexAttribute(0, 1, sizeof(glm::vec3), VK_FORMAT_R32G32B32_SFLOAT);    // layout(location = 1) in vec3 normal;
        pipeline->AddVertexAttribute(0, 2, sizeof(glm::vec3), VK_FORMAT_R32G32B32A32_SFLOAT); // layout(location = 2) in vec4 tangent;
        pipeline->AddShaderStage(fragmentShader);
        pipeline->AddShaderStage(vertexShader);
        pipeline->Assemble({swapchain->imageFormat}, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED,
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
        std::cout << "pipeline built" << std::endl;
        fragmentShader->Release();
        vertexShader->Release();

        releaseQueue.Push(scene);
        releaseQueue.Push(layout);
        releaseQueue.Push(pipeline);

        buildCmds();
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
    LoadModelSample app;
    app.appName = "Load Model Sample";
    app.title = "Load Model Sample";
    app.run();
    return 0;
}