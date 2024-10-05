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
void genTorus();
void updateData();
void generateImageResources();
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
Descriptor descriptor;
PipelineConnector connector;
GraphicsPipeline pipeline;
Compilers::GLSLCompiler compiler;
Shader fragmentShader;
Shader vertexShader;
/* objects for the creation of the normal map */
Texture normalTexture;
TextureView normalTextureView;
Sampler sampler;
Descriptor computeDescriptor;
PipelineConnector computeConnector;
ComputePipeline computePipeline;
Shader computeShader;

Camera3D camera;

struct Vertex
{
    glm::vec3 position;
};
struct DataUBO
{
    glm::mat4 projectionView;
    glm::mat4 model;
} dataUBO;
struct DataPush
{
    glm::vec3 color;
} dataPush;
std::vector<Vertex> vertices;
std::vector<uint32_t> indices;

const std::string fragmentShaderText = R"(#version 450
layout (push_constant) uniform pushData {
    vec3 color;
};
layout(location = 0) out vec4 outColor;
layout(binding = 1) uniform sampler _sampler;
layout(binding = 2) uniform texture2D _texture;

void main()
{
    outColor = vec4(color, 1.0);
})";

const std::string vertexShaderText = R"(#version 450
layout (location = 0) in vec3 pos;
layout (set = 0, binding = 0) uniform DataUBO {
    mat4 projectionView;
    mat4 model;
} ubo;

void main()
{
    gl_Position = ubo.projectionView * ubo.model * vec4(pos,1.0);
})";

const std::string computeShaderText = R"(#version 450
layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout (set = 0, binding = 0, rgba16f) uniform image2D imgOutput;

void main()
{
    float x = gl_GlobalInvocationID.x;
    float y = gl_GlobalInvocationID.y;
    float length = sqrt(x*x + y*y);
    vec4 data = vec4(0.0);
    if(length >= radiousMinor & radiousMajor >= length)
    {

    }
    imageStore(imgOutput, ivec2(gl_GlobalInvocationID.x,gl_GlobalInvocationID.y), data);
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

    // create normal map objects
    generateImageResources();

    // computeDescriptor.AddBinding(0, 0, DescriptorType::StorageImage, PipelineStage::Compute, 1);
    // computeDescriptor.Build(device);

    // computeDescriptor.AddUpdateInfo(0, 0, {normalTextureView});
    // computeDescriptor.Update();

    // computeConnector.Build(device, computeDescriptor);

    // computeShader.Create(device, compiler, computeShaderText, "main", "compute.glsl", PipelineStage::Compute, true);
    // computePipeline.Build(device, computeConnector, computeShader);

    // create rendering resources
    genTorus();
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
    descriptor.AddBinding(0, 1, DescriptorType::Sampler, PipelineStage::Fragment, 1);
    descriptor.AddBinding(0, 2, DescriptorType::SampledImage, PipelineStage::Fragment, 1);
    descriptor.Build(device);
    std::cout << "-------- descriptor created ----------" << std::endl;
    descriptor.AddUpdateInfo<DataUBO>(0, 0, {uniformBuffer});
    descriptor.AddUpdateInfo(0, 1, {sampler});
    descriptor.AddUpdateInfo(0, 2, {normalTextureView});
    descriptor.Update();
    std::cout << "-------- descriptor updated ----------" << std::endl;

    connector.AddPushConstant<DataPush>(0, PipelineStage::Fragment);
    connector.Build(device, descriptor);

    // add pipeline stuff here
    vertexShader.Create(device, compiler, vertexShaderText, "main", "vertex.glsl", PipelineStage::Vertex, true);
    fragmentShader.Create(device, compiler, fragmentShaderText, "main", "fragment.glsl", PipelineStage::Fragment, true);

    pipeline.AddVertexBindingDescription<Vertex>(0);                     // binding = 0
    pipeline.AddVertexAttribute(0, 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT); // layout(location = 0) in vec3 pos;
    pipeline.AddShaderStage(vertexShader);
    pipeline.AddShaderStage(fragmentShader);
    pipeline.Build(device, connector, renderpass, 0);
    std::cout << "-------- pipeline created ----------" << std::endl;

    vertexShader.Destroy();
    fragmentShader.Destroy();

    camera = Camera3D(width, height);
}

void generateImageResources()
{
    normalTexture.Build(device, 256, 256, 1,
                        VK_IMAGE_TYPE_2D, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, 1, 1,
                        VK_FORMAT_R8G8B8A8_SNORM, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, VK_IMAGE_LAYOUT_PREINITIALIZED);
    std::shared_ptr<Texture> pNormalTexture = std::make_shared<Texture>(normalTexture);
    normalTextureView.Build(device, pNormalTexture);
    sampler.Build(device);
    //create a fence for wait for the queue
    VkFence fence;
    VkFenceCreateInfo fenceCI = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };
    checkResult(vkCreateFence(device._device, &fenceCI, nullptr, &fence));
    vkResetFences(device._device, 1, &fence);
    // records cmd to do the transition layout
    VkCommandBufferBeginInfo beginI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    checkResult(vkBeginCommandBuffer(cmd, &beginI));
    VkImageMemoryBarrier2 barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
        .srcAccessMask = VK_ACCESS_2_NONE,
        .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
        .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED,
        .newLayout = VK_IMAGE_LAYOUT_GENERAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = normalTexture._image,
        .subresourceRange = normalTextureView._subresourceRange,
    };
    VkDependencyInfo dependencyI = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier,
    };
    vkCmdPipelineBarrier2(cmd, &dependencyI);
    checkResult(vkEndCommandBuffer(cmd));
    VkCommandBufferSubmitInfo cmdSI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = cmd,
        .deviceMask = 0,
    };
    VkSubmitInfo2 submitI = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &cmdSI,
    };
    checkResult(vkQueueSubmit2(device._graphicsQueues[0], 1, &submitI, fence));
    checkResult(vkWaitForFences(device._device, 1, &fence, true, (std::numeric_limits<uint64_t>::max)()));
    normalTexture._imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    vkDestroyFence(device._device, fence, nullptr);
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
    dataPush.color = {0.5f, 0.65f, 0.3f};
    camera.SetPosition(glm::vec3(20.0f, 20.0f, 30.0f));
    camera.SetCenter(glm::vec3(0.0f, 0.0f, 0.0f));
    camera.Update();
    dataUBO.projectionView = camera.GetProjectionView();
    dataUBO.model = glm::mat4(1.0f);

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
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {(float)width, (float)height}};
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
    descriptor.Destroy();

    sampler.Destroy();
    normalTextureView.Destroy();
    normalTexture.Destroy();

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

void genTorus()
{
    float radiusMajor = 10;
    float radiusMinor = 5;
    // float sectorStep = 10;
    float sectorCount = 30;
    // float sideStep;
    float sideCount = 15;

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