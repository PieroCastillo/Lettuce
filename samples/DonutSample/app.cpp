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

class DonutSample : public LettuceSampleApp
{
public:
    Lettuce::Core::Compilers::GLSLCompiler compiler;
    /*memory objects*/
    std::shared_ptr<Lettuce::Core::ResourcePool> hostResources;     // host cached, memory placed in host memory
    std::shared_ptr<Lettuce::Core::ResourcePool> deviceResources;   // device local, memory places in device memory
    std::shared_ptr<Lettuce::Core::ResourcePool> coherentResources; // device local|host coherent, memory placed in device memory able to be mapped
    std::shared_ptr<Lettuce::Core::TransferManager> transfer;
    uint32_t indicesSize, verticesSize, lineVerticesSize, bufferSize;
    /* sync objects*/
    std::shared_ptr<Lettuce::Core::Semaphore> renderFinished;
    /* rendering objects */
    std::shared_ptr<Lettuce::Core::BufferResource> deviceBuffer, hostBuffer; // buffer to store data

    std::shared_ptr<Lettuce::Core::BufferResource> uniformBuffer;

    std::shared_ptr<Lettuce::Core::Descriptors> descriptor;
    std::shared_ptr<Lettuce::Core::PipelineLayout> linesLayout;
    std::shared_ptr<Lettuce::Core::PipelineLayout> connector;
    std::shared_ptr<Lettuce::Core::GraphicsPipeline> pipeline;
    std::shared_ptr<Lettuce::Core::GraphicsPipeline> linespipeline;
    std::shared_ptr<Lettuce::Core::ShaderModule> fragmentShader;
    std::shared_ptr<Lettuce::Core::ShaderModule> vertexShader;
    std::shared_ptr<Lettuce::Core::ShaderModule> psLineShader;
    std::shared_ptr<Lettuce::Core::ShaderModule> vsLineShader;

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

    void onResize()
    {
    }

    void createObjects()
    {
        renderFinished = std::make_shared<Lettuce::Core::Semaphore>(device, 0);

        buildCmds();
        genTorus();

        hostResources = std::make_shared<Lettuce::Core::ResourcePool>();
        deviceResources = std::make_shared<Lettuce::Core::ResourcePool>();
        transfer = std::make_shared<TransferManager>(device);

        // buffer  blocks layout:
        // |                  buffer                     |
        // |  vertex  |   index  | line1 | line2 | line3 |
        indicesSize = indices.size() * sizeof(uint32_t);
        verticesSize = vertices.size() * sizeof(Vertex);
        lineVerticesSize = 6 * sizeof(LineVertex);
        bufferSize = lineVerticesSize + indicesSize + verticesSize;

        // create buffers
        hostBuffer = std::make_shared<BufferResource>(device, bufferSize, VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT);
        deviceBuffer = std::make_shared<BufferResource>(device, bufferSize, VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_2_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_2_TRANSFER_DST_BIT);

        // add buffers to their respectives pools
        hostResources->AddResource(hostBuffer);
        deviceResources->AddResource(deviceBuffer);

        // bind resources
        hostResources->Bind(device, VK_MEMORY_PROPERTY_HOST_CACHED_BIT);    // create pool in host memory
        deviceResources->Bind(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); // create pool in device memory

        // write host buffer
        void *data = malloc(bufferSize); // create a temporal pointer to write data
        uint8_t *ptr = static_cast<uint8_t *>(data);
        memcpy(ptr, vertices.data(), verticesSize);
        ptr += verticesSize;

        memcpy(ptr, indices.data(), indicesSize);
        ptr += indicesSize;

        std::vector<glm::vec3> lines = {glm::vec3(0), glm::vec3(100, 0, 0), glm::vec3(0), glm::vec3(0, 100, 0), glm::vec3(0), glm::vec3(0, 0, 100)};
        memcpy(ptr, lines.data(), lineVerticesSize);
        // transfer memory to memory allocation
        hostResources->Map(0, bufferSize);
        hostResources->SetData(data, 0, bufferSize);
        hostResources->UnMap();
        free(data); // release the temporal pointer

        // transfer resources from host to device memory
        transfer->Prepare();
        transfer->AddTransference(hostBuffer, deviceBuffer, TransferType::HostToDevice);
        transfer->TransferAll();

        // release host resources
        hostBuffer->Release();
        hostResources->Release();

        // create coherent resources
        coherentResources = std::make_shared<ResourcePool>();
        uniformBuffer = std::make_shared<BufferResource>(device, sizeof(DataUBO), VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT);
        coherentResources->AddResource(uniformBuffer);
        coherentResources->Bind(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        // write coherent buffer
        coherentResources->Map(0, sizeof(DataUBO));

        /*setup stuff to render the donut*/
        descriptor = std::make_shared<Lettuce::Core::Descriptors>(device);
        descriptor->AddBinding(0, 0, DescriptorType::UniformBuffer, PipelineStage::Vertex, 1);
        descriptor->Assemble();
        descriptor->AddUpdateInfo(0, 0, {{sizeof(DataUBO), uniformBuffer}});
        descriptor->Update();

        connector = std::make_shared<Lettuce::Core::PipelineLayout>(device, descriptor);
        connector->AddPushConstant<DataPush>(VK_SHADER_STAGE_FRAGMENT_BIT);
        connector->Assemble();
        // add pipeline stuff here
        vertexShader = std::make_shared<Lettuce::Core::ShaderModule>(device, compiler, vertexShaderText, "main", "vertex.glsl", PipelineStage::Vertex, true);
        fragmentShader = std::make_shared<Lettuce::Core::ShaderModule>(device, compiler, fragmentShaderText, "main", "fragment.glsl", PipelineStage::Fragment, true);

        pipeline = std::make_shared<GraphicsPipeline>(device, connector);
        pipeline->AddVertexBindingDescription<Vertex>(0);                                  // binding = 0
        pipeline->AddVertexAttribute(0, 0, 0, VK_FORMAT_R32G32B32_SFLOAT);                 // layout(location = 0) in vec3 pos;
        pipeline->AddVertexAttribute(0, 1, sizeof(glm::vec3), VK_FORMAT_R32G32B32_SFLOAT); // layout(location = 1) in vec3 normal;
        pipeline->AddShaderStage(vertexShader);
        pipeline->AddShaderStage(fragmentShader);
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

        vertexShader->Release();
        fragmentShader->Release();

        /*setup pipeline for lines*/
        vsLineShader = std::make_shared<ShaderModule>(device, compiler, vsLineShaderText, "main", "vsLine.glsl", PipelineStage::Vertex, true);
        psLineShader = std::make_shared<ShaderModule>(device, compiler, psLineShaderText, "main", "psLine.glsl", PipelineStage::Fragment, true);

        linesLayout = std::make_shared<PipelineLayout>(device, descriptor);
        linesLayout->AddPushConstant<DataPush>(VK_SHADER_STAGE_FRAGMENT_BIT);
        linesLayout->Assemble();

        linespipeline = std::make_shared<GraphicsPipeline>(device, linesLayout);

        linespipeline->AddVertexBindingDescription<LineVertex>(0);
        linespipeline->AddVertexAttribute(0, 0, 0, VK_FORMAT_R32G32B32_SFLOAT);
        linespipeline->AddShaderStage(vsLineShader);
        linespipeline->AddShaderStage(psLineShader);
        linespipeline->Assemble({swapchain->imageFormat}, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED,
                                {.inputAssembly = {VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_FALSE},
                                 .rasterization = {
                                     .frontFace = VK_FRONT_FACE_CLOCKWISE,
                                 },
                                 .colorBlend = {
                                     .attachments = {
                                         {
                                             .colorWriteMask = VK_COMPONENT_SWIZZLE_R | VK_COMPONENT_SWIZZLE_G | VK_COMPONENT_SWIZZLE_B | VK_COMPONENT_SWIZZLE_A,
                                         },
                                     },
                                 }});

        psLineShader->Release();
        vsLineShader->Release();

        std::cout << "objects created successfully" << std::endl;

        releaseQueue.Push(renderFinished);
        releaseQueue.Push(transfer);
        releaseQueue.PushWithBefore(coherentResources, [&coherentResources = this->coherentResources]()
                                    { coherentResources->UnMap(); });
        releaseQueue.Push(deviceResources);
        releaseQueue.Push(uniformBuffer);
        releaseQueue.Push(deviceBuffer);
        releaseQueue.Push(descriptor);
        releaseQueue.Push(connector);
        releaseQueue.Push(pipeline);
        releaseQueue.Push(linesLayout);
        releaseQueue.Push(linespipeline);

        camera.SetProjection(width, height);// = Lettuce::X3D::Camera3D::Camera3D(width, height);
        camera.Position = glm::vec3(20, 20, 30);
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
            .level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        checkResult(vkAllocateCommandBuffers(device->GetHandle(), &cmdAI, &cmd));
    }

    void updateData()
    {
        dataPush.color = glm::vec3(1.0f, 0.5f, 0.31f);
        // camera.SetPosition(glm::vec3(20, 20, 30));
        // camera.Update();
        dataUBO.projectionView = camera.GetProjectionViewMatrix();
        dataUBO.model = glm::mat4(1.0f);
        dataUBO.cameraPos = camera.Position;// .eye;
        // dataUBO.cameraPos = glm::vec3(30);

        coherentResources->SetData(&dataUBO, 0, sizeof(DataUBO));
    }
    void recordCmds()
    {
        // rendering cmd

        VkImageSubresourceRange imgSubresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

        checkResult(vkResetCommandBuffer(cmd, 0));

        VkClearValue clearValue;
        clearValue.color = {{0.5f, 0.5f, 0.5f, 1.0f}};

        VkCommandBufferBeginInfo cmdBI = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
        };
        checkResult(vkBeginCommandBuffer(cmd, &cmdBI));

        VkImageMemoryBarrier2 imageBarrier2 = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
            .dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
            .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
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

        /*render lines X Y Z */ // yellow, purple, green
        uint32_t baseSize = verticesSize + indicesSize;
        std::vector<std::pair<uint32_t, glm::vec3>> pairs = {
            {baseSize, glm::vec3(1, 0.984, 0)},
            {baseSize + (2 * sizeof(LineVertex)), glm::vec3(0.506, 0.024, 0.98)},
            {baseSize + (4 * sizeof(LineVertex)), glm::vec3(0.024, 0.98, 0.173)},
        };
        for (auto &[offset, color] : pairs)
        {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, linespipeline->GetHandle());
            VkDeviceSize _offset = offset;
            VkDeviceSize size = 2 * sizeof(LineVertex);
            vkCmdBindVertexBuffers2(cmd, 0, 1, deviceBuffer->GetHandlePtr(), &_offset, &size, nullptr);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, linesLayout->GetHandle(), 0, 1, descriptor->_descriptorSets.data(), 0, nullptr);
            dataPush.color = color;
            vkCmdPushConstants(cmd, linesLayout->GetHandle(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DataPush), &dataPush);
            vkCmdSetLineWidth(cmd, 5.0f);
            VkViewport viewport = {0, 0, (float)width, (float)height, 0.0f, 1.0f};
            vkCmdSetViewportWithCount(cmd, 1, &viewport);
            VkRect2D scissor = {{0, 0}, {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}};
            vkCmdSetScissorWithCount(cmd, 1, &scissor);
            vkCmdSetPrimitiveTopology(cmd, VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
            vkCmdDraw(cmd, 2, 1, 0, 0);
        }
        /*render donut*/ 
        // TODO: find error
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetHandle());
        VkDeviceSize size = verticesSize;
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers2(cmd, 0, 1, deviceBuffer->GetHandlePtr(), &offset, &size, nullptr);
        // vkCmdBindIndexBuffer2(cmd, deviceBuffer->GetHandle(), verticesSize, indicesSize, VK_INDEX_TYPE_UINT32);
        vkCmdBindIndexBuffer(cmd, deviceBuffer->GetHandle(), verticesSize, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, connector->GetHandle(), 0, 1, descriptor->_descriptorSets.data(), 0, nullptr);

        vkCmdPushConstants(cmd, connector->GetHandle(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DataPush), &dataPush);
        vkCmdSetLineWidth(cmd, 1.0f);
        VkViewport viewport = {0, 0, (float)width, (float)height, 0.0f, 1.0f};
        // vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetViewportWithCount(cmd, 1, &viewport);
        VkRect2D scissor = {{0, 0}, {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}};
        // vkCmdSetScissor(cmd, 0, 1, &scissor);
        vkCmdSetScissorWithCount(cmd, 1, &scissor);
        vkCmdSetPrimitiveTopology(cmd, VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        vkCmdDrawIndexed(cmd, indices.size(), 1, 0, 0, 0);

        vkCmdEndRendering(cmd);

        imageBarrier2.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        imageBarrier2.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        imageBarrier2.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        imageBarrier2.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
        imageBarrier2.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageBarrier2.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
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
        camera.SetProjection(width, height);// = Lettuce::X3D::Camera3D::Camera3D(width, height);
        // camera.Reset(glm::vec3(20, 20, 30), glm::vec3(0.0f), glm::vec3(0.57735026919)); // 1 / sqrt(3)
    }

    void destroyObjects()
    {
        vkFreeCommandBuffers(device->GetHandle(), pool, 1, &cmd);
        vkDestroyCommandPool(device->GetHandle(), pool, nullptr);
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
    DonutSample app;
    app.appName = "Donut Sample";
    app.title = "Donut Sample";
    app.features = {
        .FragmentShadingRate = false,
        .ExecutionGraphs = false,
        .MeshShading = false,
        .RayTracing = false,
        .Video = false,
    };
    app.run();
    return 0;
}