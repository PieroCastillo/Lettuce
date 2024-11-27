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

class LoadModel : public LettuceSampleApp
{
public:
    Lettuce::Core::RenderPass renderpass;
    /* sync objects*/
    Lettuce::Core::Semaphore renderFinished;
    /* rendering objects */
    Lettuce::Core::Buffer vertexBuffer;
    Lettuce::Core::Buffer indexBuffer;

    Lettuce::Core::Buffer LineBuffer1;
    Lettuce::Core::Buffer LineBuffer2;
    Lettuce::Core::Buffer LineBuffer3;

    Lettuce::Core::Buffer uniformBuffer;
    Lettuce::Core::Descriptors descriptor;
    Lettuce::Core::PipelineLayout linesLayout;
    Lettuce::Core::PipelineLayout connector;
    Lettuce::Core::GraphicsPipeline pipeline;
    Lettuce::Core::GraphicsPipeline linesPipeline;
    Lettuce::Core::Compilers::GLSLCompiler compiler;
    Lettuce::Core::Shader fragmentShader;
    Lettuce::Core::Shader vertexShader;
    Lettuce::Core::Shader psLineShader;
    Lettuce::Core::Shader vsLineShader;

    Lettuce::X3D::Camera3D camera;

    struct LineVertex
    {
        glm::vec3 position;
    };

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

    const std::string psLineShaderText = R"(#version 450
layout (push_constant) uniform pushData {
    vec3 color;
} pushdata;
layout (location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(pushdata.color,1.0);
})";

    const std::string vsLineShaderText = R"(#version 450
layout (location=0) in vec3 pos;
layout (set = 0, binding = 0) uniform DataUBO {
    mat4 projectionView;
    mat4 model;
    vec3 cameraPos;
} ubo;

void main()
{
    gl_Position = ubo.projectionView * ubo.model * vec4(pos,1.0);
})";

    const std::string fragmentShaderText = R"(#version 450
layout (push_constant) uniform pushData {
    vec3 color;
} data;
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

    void createRenderPass()
    {
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
        for (auto &view : swapchain.swapchainTextureViews)
        {
            renderpass.AddFramebuffer(width, height, {view});
        }
        renderpass.BuildFramebuffers();
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

    double sensibility = 0.01f;
    void onCursorMotion(double xpos, double ypos)
    {
        if (pressed)
        {
            double dx = xl - xpos;
            double dy = yl - ypos;

            camera.MoveByMouse(dx, dy, sensibility);
            std::cout << "moved by mouse" << std::endl;
        }
    }

    void createObjects()
    {
        renderFinished.Create(device, 0);
        buildCmds();
        genTorus();
        vertexBuffer = Buffer::CreateVertexBuffer(device, vertices);
        indexBuffer = Buffer::CreateIndexBuffer(device, indices);

        uniformBuffer = Buffer::CreateUniformBuffer<DataUBO>(device);
        uniformBuffer.Map();
        uniformBuffer.SetData(&dataUBO);
        /*setup stuff to render the donut*/
        descriptor.AddBinding(0, 0, DescriptorType::UniformBuffer, PipelineStage::Vertex, 1);
        descriptor.Build(device);
        descriptor.AddUpdateInfo<DataUBO>(0, 0, uniformBuffer);
        descriptor.Update();

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

        vertexShader.Destroy();
        fragmentShader.Destroy();
        /*setup line buffers*/

        LineBuffer1 = Buffer::CreateVertexBuffer<LineVertex>(device, {{glm::vec3(0)}, {glm::vec3(100, 0, 0)}});
        LineBuffer2 = Buffer::CreateVertexBuffer<LineVertex>(device, {{glm::vec3(0)}, {glm::vec3(0, 100, 0)}});
        LineBuffer3 = Buffer::CreateVertexBuffer<LineVertex>(device, {{glm::vec3(0)}, {glm::vec3(0, 0, 100)}});

        /*setup pipeline for lines*/
        vsLineShader.Create(device, compiler, vsLineShaderText, "main", "vsLine.glsl", PipelineStage::Vertex, true);
        psLineShader.Create(device, compiler, psLineShaderText, "main", "psLine.glsl", PipelineStage::Fragment, true);

        linesLayout.AddPushConstant<DataPush>(0, PipelineStage::Fragment);
        linesLayout.Build(device, descriptor);

        linesPipeline.AddVertexBindingDescription<LineVertex>(0);
        linesPipeline.AddVertexAttribute(0, 0, 0, VK_FORMAT_R32G32B32_SFLOAT);
        linesPipeline.AddShaderStage(vsLineShader);
        linesPipeline.AddShaderStage(psLineShader);

        linesPipeline.Build(device, linesLayout, renderpass, 0,
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
        psLineShader.Destroy();
        vsLineShader.Destroy();
        camera = Lettuce::X3D::Camera3D::Camera3D(width, height);
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
        // camera.SetPosition(glm::vec3(20, 20, 30));
        camera.SetPosition(cameraPosition);
        camera.SetCenter(glm::vec3(0.0f, 0.0f, 0.0f));
        camera.Update();
        dataUBO.projectionView = camera.GetProjectionView();
        dataUBO.model = glm::mat4(1.0f);
        dataUBO.cameraPos = cameraPosition;
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

        /*render lines X Y Z */ // yellow, purple, green
        std::vector<std::pair<Buffer, glm::vec3>> pairs = {{LineBuffer1, glm::vec3(1, 0.984, 0)}, {LineBuffer2, glm::vec3(0.506, 0.024, 0.98)}, {LineBuffer3, glm::vec3(0.024, 0.98, 0.173)}};
        for (auto &[lineBuffer, color] : pairs)
        {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, linesPipeline._pipeline);
            VkDeviceSize size = 0;
            vkCmdBindVertexBuffers(cmd, 0, 1, &(lineBuffer._buffer), &size);
            // vkCmdBindIndexBuffer(cmd, indexBuffer._buffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, linesLayout._pipelineLayout, 0, 1, descriptor._descriptorSets.data(), 0, nullptr);
            dataPush.color = color;
            vkCmdPushConstants(cmd, linesLayout._pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DataPush), &dataPush);
            vkCmdSetLineWidth(cmd, 5.0f);
            VkViewport viewport = {0, 0, (float)width, (float)height, 0.0f, 1.0f};
            // vkCmdSetViewport(cmd, 0, 1, &viewport);
            vkCmdSetViewportWithCount(cmd, 1, &viewport);
            VkRect2D scissor = {{0, 0}, {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}};
            // vkCmdSetScissor(cmd, 0, 1, &scissor);
            vkCmdSetScissorWithCount(cmd, 1, &scissor);
            vkCmdSetPrimitiveTopology(cmd, VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
            vkCmdDraw(cmd, 2, 1, 0, 0);
            // vkCmdDrawIndexed(cmd, indices.size(), 1, 0, 0, 0);
        }
        /*render donut*/
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
        VkRect2D scissor = {{0, 0}, {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}};
        // vkCmdSetScissor(cmd, 0, 1, &scissor);
        vkCmdSetScissorWithCount(cmd, 1, &scissor);
        vkCmdSetPrimitiveTopology(cmd, VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        vkCmdDrawIndexed(cmd, indices.size(), 1, 0, 0, 0);
        vkCmdEndRenderPass(cmd);

        checkResult(vkEndCommandBuffer(cmd));
    }
    uint64_t renderFinishedValue = 0;
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

    void beforeResize()
    {
        camera = Lettuce::X3D::Camera3D::Camera3D(width, height);
    }

    void destroyObjects()
    {
        vkFreeCommandBuffers(device._device, pool, 1, &cmd);
        vkDestroyCommandPool(device._device, pool, nullptr);

        linesPipeline.Destroy();
        linesLayout.Destroy();
        LineBuffer1.Destroy();
        LineBuffer2.Destroy();
        LineBuffer3.Destroy();

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

                float nx = std::cosf(phi) * std::cosf(theta);
                float ny = std::cosf(phi) * std::sinf(theta);
                float nz = std::sinf(phi);

                vertices.push_back({{x, y, z}, {nx, ny, nz}});
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
};

int main()
{
    LoadModel app;
    app.features = {
        .FragmentShadingRate = false,
        .PresentWait = false,
        .ExecutionGraphs = false,
        .MeshShading = false,
        .RayTracing = false,
        .Video = false,
        .MemoryBudget = false,
        .ConditionalRendering = false,
        .DescriptorBuffer = false,
    };
    app.run();
    return 0;
}