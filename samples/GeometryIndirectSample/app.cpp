//
// Created by piero on 17/11/2024.
//
#include "LettuceSampleApp.hpp"
#include "Lettuce/Lettuce.X3D.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <numbers>
#include <tuple>
#include <memory>
#include <string>

using namespace Lettuce::Core;
using namespace Lettuce::X3D;

class GeometrySample : public LettuceSampleApp
{
public:
    Lettuce::Core::Compilers::GLSLCompiler compiler;
    std::shared_ptr<Lettuce::Core::RenderPass> renderpass;
    std::shared_ptr<TransferManager> manager;
    /* sync objects*/
    std::shared_ptr<Lettuce::Core::Semaphore> renderFinished;
    /* rendering objects */
    std::shared_ptr<Sampler> sampler;
    std::shared_ptr<Descriptors> descriptors_bindable;
    std::shared_ptr<PipelineLayout> layout_bindable;
    std::vector<std::shared_ptr<Shader>> shaders_bindable;
    std::shared_ptr<ResourcePool> pool_images;
    std::vector<std::shared_ptr<ImageResource>> images_bindable;
    std::vector<std::shared_ptr<ImageViewResource>> imgVw_bindable;
    /* required stuff to generate commands*/
    std::shared_ptr<Descriptors> descriptors_genCommands;
    std::shared_ptr<PipelineLayout> layout_genCommands;
    std::shared_ptr<ComputePipeline> pipeline_genCommands; // generate the commands
    std::shared_ptr<ResourcePool> pool_generatedCommands;
    std::shared_ptr<ResourcePool> pool_preprocessCommands;
    std::shared_ptr<ResourcePool> pool_ubo;
    std::shared_ptr<BufferResource> buffer_generatedCommands;
    std::shared_ptr<BufferResource> buffer_preprocessCommands;
    std::shared_ptr<BufferResource> buffer_ubo;
    std::shared_ptr<IndirectCommandsLayout> indirectCommandsLayout;
    std::shared_ptr<IndirectExecutionSet> indirectExecutionSet;

    uint32_t address_generatedCommands;
    uint32_t address_preprocessCommands;
    const uint32_t sequences = 16;
    const uint32_t draws = 16;
    const std::string compShaderText = "";
    const std::string vertexShaderText = "";
    const std::string frag1ShaderText = "";
    const std::string frag2ShaderText = "";
    const std::string frag3ShaderText = "";
    const std::string frag4ShaderText = "";
    struct imgData
    {
        int texWidth, texHeight, texChannels;
        stbi_uc *pixels;
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

    void createImages()
    {
        /*
        - load images
        - copy images to staging images
        - transfer to device-allocated images
        */
        auto pool_temp = std::make_shared<ResourcePool>();
        pool_images = std::make_shared<ResourcePool>();
        std::vector<std::string> filenames = {"assets/texture1.jpg", "assets/texture2.jpg", "assets/texture3.jpg", "assets/texture4.jpg"};
        // std::vector<imgData> imgDatas;
        std::vector<std::shared_ptr<BufferResource>> imgs_temp;
        std::vector<std::pair<void *, uint32_t>> imgPtrs;
        for (int i = 0; i < 4; i++)
        {
            int texWidth, texHeight, texChannels;
            if (!std::filesystem::exists(filenames[i]))
            {
                std::cout << " file " << filenames[i] << " does not exist" << std::endl;
                std::abort();
            }
            stbi_uc *pixels = stbi_load(filenames[i].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

            if (!pixels)
            {
                std::abort();
            }
            // imgDatas.push_back({texWidth, texHeight, texChannels, pixels});
            imgPtrs.push_back({(void *)pixels, 4 * texHeight * texWidth});

            auto img_temp = std::make_shared<BufferResource>(device, 4 * texHeight * texWidth, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

            auto img_dst = std::make_shared<ImageResource>(device,
                                                           (uint32_t)texWidth,
                                                           (uint32_t)texHeight,
                                                           1, VkImageType::VK_IMAGE_TYPE_2D, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                           1, 1, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_PREINITIALIZED);

            imgs_temp.push_back(img_temp);
            images_bindable.push_back(img_dst);
            pool_temp->AddResource(img_temp);
            pool_images->AddResource(img_dst);
        }

        pool_temp->Bind(device, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        pool_images->Bind(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        std::cout << "all binded" << std::endl;

        auto [dataPtr, sz] = AllocAllInOne(imgPtrs);
        pool_temp->Map(0, pool_temp->GetSize());
        pool_temp->SetData(dataPtr, 0, sz);
        pool_temp->UnMap();
        std::cout << "images copied to host coherent memory" << std::endl;

        manager->Prepare();
        for (int i = 0; i < 4; i++)
        {
            manager->AddTransference(imgs_temp[i], images_bindable[i], 0, TransferType::HostToDevice);
        }
        manager->TransferAll();
        std::cout << "images transfered" << std::endl;
        for (auto &img : images_bindable)
        {
            auto vw = std::make_shared<ImageViewResource>(device, img);
            imgVw_bindable.push_back(vw);
        }

        free(dataPtr);
        for (auto &[ptr, _] : imgPtrs)
        {
            free(ptr);
        }
        for (auto &img : imgs_temp)
        {
            img->Release();
        }
        pool_temp->Release();

        releaseQueue.Push(pool_images);
        for (auto &img : images_bindable)
        {
            releaseQueue.Push(img);
        }
        for (auto &imgVw : imgVw_bindable)
        {
            releaseQueue.Push(imgVw);
        }
    }

    void createSampler()
    {
        sampler = std::make_shared<Sampler>(device);
        releaseQueue.Push(sampler);
    }

    void createIndirectCommandsGenerator()
    {
        descriptors_genCommands = std::make_shared<Descriptors>(device);
        descriptors_genCommands->AddBinding(0, 0, DescriptorType::StorageBuffer, PipelineStage::Compute, 1); // indirect buffer
        descriptors_genCommands->Assemble();
        descriptors_genCommands->AddUpdateInfo(0, 0, {{buffer_generatedCommands->_size, buffer_generatedCommands}});
        descriptors_genCommands->Update();
        layout_genCommands = std::make_shared<PipelineLayout>(device, descriptors_genCommands);
        layout_genCommands->Assemble();

        auto compShader = std::make_shared<ShaderModule>(device, compiler, compShaderText, "main", "compute.glsl", PipelineStage::Compute, true);
        pipeline_genCommands = std::make_shared<ComputePipeline>(device, layout_genCommands, compShader);
        compShader->Release();

        releaseQueue.Push(descriptors_genCommands);
        releaseQueue.Push(layout_genCommands);
        releaseQueue.Push(pipeline_genCommands);
    }

    void createIndirectObjects()
    {
        descriptors_bindable = std::make_shared<Descriptors>(device);
        descriptors_bindable->AddBinding(0, 0, DescriptorType::UniformBuffer, PipelineStage::Vertex, 1);
        descriptors_bindable->AddBinding(0, 1, DescriptorType::SampledImage, PipelineStage::Fragment, 1);
        descriptors_bindable->Assemble();
        descriptors_bindable->AddUpdateInfo(0, 0, {{buffer_ubo->_size, buffer_ubo}});
        descriptors_bindable->AddUpdateInfo(0, 1, {
                                                      {sampler, imgVw_bindable[0]},
                                                      {sampler, imgVw_bindable[1]},
                                                      {sampler, imgVw_bindable[2]},
                                                      {sampler, imgVw_bindable[3]},
                                                  });
        descriptors_bindable->Update();
        layout_bindable = std::make_shared<PipelineLayout>(device, descriptors_bindable);
        layout_bindable->Assemble();

        const std::vector<std::string> shadersText = {frag1ShaderText, frag2ShaderText, frag3ShaderText, frag4ShaderText};
        // melocotón pastel, amarillo pastel, rosa suave pastel, salmón claro pastel
        const std::vector<glm::vec3> shadersConst = {{0.98, 0.75, 0.65}, {0.95, 0.82, 0.60}, {0.99, 0.70, 0.70}, {0.92, 0.68, 0.55}};

        auto vertShader = std::make_shared<Shader>(device, layout_bindable);
        auto codeV = compiler.Compile(vertexShaderText, "vert.glsl", PipelineStage::Vertex, true);
        vertShader->Assemble(VK_SHADER_STAGE_VERTEX_BIT, codeV);
        shaders_bindable.push_back(vertShader);

        for (int i = 1; i <= 4; i++)
        {
            auto fragShader = std::make_shared<Shader>(device, layout_bindable);
            auto code = compiler.Compile(shadersText[i], "frag_" + std::to_string(i), PipelineStage::Fragment, true);
            fragShader->AddConstant(0, sizeof(glm::vec3), (void *)&shadersConst[i]);
            fragShader->Assemble(VK_SHADER_STAGE_FRAGMENT_BIT, code);
            shaders_bindable.push_back(fragShader);
        }

        indirectCommandsLayout = std::make_shared<IndirectCommandsLayout>(device, layout_bindable);
        indirectCommandsLayout->AddExecutionSetToken(VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 2); // bound to 2 shaders
        indirectCommandsLayout->AddVertexBufferToken(0);
        indirectCommandsLayout->AddIndexBufferToken();
        indirectCommandsLayout->AddDrawIndexedToken();
        indirectCommandsLayout->Assemble(VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, false);

        indirectExecutionSet = std::make_shared<IndirectExecutionSet>(device, layout_bindable);
        indirectExecutionSet->Assemble(shaders_bindable, (uint32_t)shaders_bindable.size());
        auto reqs = indirectExecutionSet->GetRequirements(indirectCommandsLayout, sequences, draws);

        buffer_preprocessCommands = std::make_shared<BufferResource>(device, reqs.size, VK_BUFFER_USAGE_2_PREPROCESS_BUFFER_BIT_EXT);
        pool_preprocessCommands = std::make_shared<ResourcePool>();
        pool_preprocessCommands->AddResource(buffer_preprocessCommands);
        pool_preprocessCommands->Bind(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, reqs.memoryTypeBits);
        address_preprocessCommands = buffer_preprocessCommands->GetAddress();

        buffer_generatedCommands = std::make_shared<BufferResource>(device, sequences * indirectCommandsLayout->GetSize(), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        pool_generatedCommands = std::make_shared<ResourcePool>();
        pool_generatedCommands->AddResource(buffer_generatedCommands);
        pool_generatedCommands->Bind(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        address_generatedCommands = buffer_generatedCommands->GetAddress();

        releaseQueue.Push(descriptors_bindable);
        releaseQueue.Push(layout_bindable);
        for (auto &shader : shaders_bindable)
        {
            releaseQueue.Push(shader);
        }
        releaseQueue.Push(indirectCommandsLayout);
        releaseQueue.Push(indirectExecutionSet);
        releaseQueue.Push(buffer_preprocessCommands);
        releaseQueue.Push(pool_preprocessCommands);
        releaseQueue.Push(buffer_generatedCommands);
        releaseQueue.Push(pool_generatedCommands);
    }
    void createObjects()
    {
        renderFinished = std::make_shared<Lettuce::Core::Semaphore>(device, 0);
        releaseQueue.Push(renderFinished);

        manager = std::make_shared<TransferManager>(device);
        releaseQueue.Push(manager);
        buildCmds();
        genScene();
        createImages();
        createSampler();
        // createIndirectCommandsGenerator();
        // createIndirectObjects();

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

        // gen commands
        /*
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, layout_genCommands->_pipelineLayout, 0,
                                        (uint32_t)descriptors_genCommands->_descriptorSets.size(),
                                        descriptors_genCommands->_descriptorSets.data(), 0, nullptr);
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_genCommands->_pipeline);
                vkCmdDispatch(cmd, 1, 1, 0); // workgroup (4,4,0)
        */
        // execute commands
        /*
                VkGeneratedCommandsInfoEXT genCommandsI = {
                    .sType = VK_STRUCTURE_TYPE_GENERATED_COMMANDS_INFO_EXT,
                    .shaderStages = indirectCommandsLayout->_shaderStages,
                    .indirectExecutionSet = indirectExecutionSet->_executionSet,
                    .indirectCommandsLayout = indirectCommandsLayout->_commandsLayout,
                    .indirectAddress = (VkDeviceAddress)address_generatedCommands,
                    .indirectAddressSize = (VkDeviceSize)buffer_generatedCommands->_size,
                    .preprocessAddress = (VkDeviceAddress)address_preprocessCommands,
                    .preprocessSize = (VkDeviceSize)buffer_preprocessCommands->_size,
                    .maxSequenceCount = 16,
                    // .sequenceCountAddress;
                    .maxDrawCount = 16,
                };

        vkCmdExecuteGeneratedCommandsEXT(cmd, VK_FALSE, &genCommandsI);
 */
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
    app.appName = "Geometry Indirect Sample";
    app.title = "Geometry Indirect Sample";
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