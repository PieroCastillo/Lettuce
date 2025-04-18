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
    std::shared_ptr<ResourcePool> pool_geometry;
    std::shared_ptr<BufferResource> buffer_generatedCommands;
    std::shared_ptr<BufferResource> buffer_preprocessCommands;
    std::shared_ptr<BufferResource> buffer_ubo;
    std::shared_ptr<BufferResource> buffer_geometry;
    std::shared_ptr<IndirectCommandsLayout> indirectCommandsLayout;
    std::shared_ptr<IndirectExecutionSet> indirectExecutionSet;

    uint64_t address_generatedCommands;
    uint64_t address_preprocessCommands;
    struct PushConstantData_Gen
    {
        uint64_t address_geometry;
        int vertexCount;
        int indexCount;
    } pcData_gen;
    const uint32_t sequences = 16;
    const uint32_t draws = 16;
    const std::string compShaderText = R"(#version 460
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_scalar_block_layout : enable
        
struct VkBindVertexBufferIndirectCommandEXT {
        uint64_t bufferAddress;
        uint size;
        uint stride;
};

struct VkBindIndexBufferIndirectCommandEXT {
        uint64_t bufferAddress;
        uint size;
        uint indexType; // 32-bit uint
};

struct VkDrawIndexedIndirectCommand {
        uint indexCount;
        uint instanceCount;
        uint firstIndex;
        int vertexOffset;
        uint firstInstance;
};

struct Sequence {
        uint shaderIndex1;
        uint shaderIndex2;
        int pushValue; // texture index
        VkBindVertexBufferIndirectCommandEXT vertex0;
        VkBindVertexBufferIndirectCommandEXT vertex1;
        VkBindIndexBufferIndirectCommandEXT index;
        VkDrawIndexedIndirectCommand draw;
};

layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;

layout(scalar, push_constant) uniform constants {
    uint64_t address;
    int indexCount;
    int vertexCount;
} consts;

layout(scalar, set = 0, binding = 0) buffer writeonly IndirectBuffer {
    Sequence sequences[];
} ind;

void main(){        
    Sequence seq;
    seq.shaderIndex1 = 0; // selects first shader always
    seq.shaderIndex2 = gl_LocalInvocationID.x; // selects fragment shader

    seq.pushValue = int(gl_LocalInvocationID.y); // selects image

    seq.vertex0.bufferAddress = consts.address;
    seq.vertex0.size = 12; // sizeof(vec3) 
    seq.vertex0.stride = 12+8;

    seq.vertex1.bufferAddress = consts.address;
    seq.vertex1.size = 8; // sizeof(vec2) 
    seq.vertex1.stride = 12+8;

    seq.index.bufferAddress = consts.address + ((12+8)*consts.vertexCount);
    seq.index.size = 4; //sizeof(uint32)
    seq.index.indexType = 1;

    seq.draw.indexCount = consts.indexCount;
    seq.draw.instanceCount = 1;
    seq.draw.firstIndex = 0;
    seq.draw.vertexOffset = 0;
    seq.draw.firstInstance = 0;
    
    uint index = (4 * gl_LocalInvocationID.y) + gl_LocalInvocationID.x - 1;
    ind.sequences[index] = seq;
})";

    const std::string vertexShaderText = R"(#version 450
#extension GL_EXT_scalar_block_layout : enable
layout (location=0) in vec3 pos;
layout (location=1) in vec2 tex;

layout (location=0) out vec2 fragTexCoord;

layout (std430, set = 0, binding = 0) uniform DataUBO {
    mat4 projectionView;
    mat4 model;
    vec3 cameraPos;
} ubo;

void main()
{
    fragTexCoord = tex;
    gl_Position = ubo.projectionView * ubo.model * vec4(pos,1.0);
})";

    const std::string fragShaderText = R"(#version 450  
#extension GL_EXT_nonuniform_qualifier : require
layout (location = 0) in vec2 fragTexCoord;
layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 1) uniform sampler2D texSampler[];

layout (push_constant) uniform constants {
    int index;
} consts;

layout (constant_id = 0) const double importantColorR = .5;
layout (constant_id = 1) const double importantColorG = .5;
layout (constant_id = 2) const double importantColorB = .5;

void main()
{
    vec3 importantColor = vec3(importantColorR, importantColorG, importantColorB);
    // outColor = vec4(importantColor  * texture(texSampler[consts.index], fragTexCoord).rgb, 1.0);
    outColor = vec4(1.0,1.0,1.0,1.0);
})";

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

    void onResize()
    {
    }

    void genScene()
    {
        /* buffer with geometry info:
            || vec3 | vec2  || ... || uint32 || uint32 ||
            first vertices, next indices
        */

        /*geometries*/
        Geometries::Sphere sphere({0, 0, 0}, 10.0f, 5, 5);

        auto poolTemp = std::make_shared<ResourcePool>();
        pool_geometry = std::make_shared<ResourcePool>();

        auto fullSize = sphere.info.indexBlock.size + sphere.info.vertBlock.size;
        auto bufferTemp = std::make_shared<BufferResource>(device, fullSize, VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT);
        buffer_geometry = std::make_shared<BufferResource>(device, fullSize, VK_BUFFER_USAGE_2_TRANSFER_DST_BIT | VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_2_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT);

        poolTemp->AddResource(bufferTemp);
        pool_geometry->AddResource(buffer_geometry);

        poolTemp->Bind(device, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        pool_geometry->Bind(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);

        auto [alloc, sz] = AllocAllInOne({{(void *)sphere.points.data(), sphere.info.vertBlock.size},
                                          {(void *)sphere.indices.data(), sphere.info.indexBlock.size}});

        poolTemp->Map(0, fullSize);
        poolTemp->SetData(alloc, 0, fullSize);
        poolTemp->UnMap();

        manager->Prepare();
        manager->AddTransference(bufferTemp, buffer_geometry, TransferType::HostToDevice);
        manager->TransferAll();

        bufferTemp->Release();
        poolTemp->Release();

        pcData_gen.address_geometry = buffer_geometry->GetAddress();
        pcData_gen.indexCount = sphere.indices.size();
        pcData_gen.vertexCount = sphere.points.size();

        releaseQueue.Push(buffer_geometry);
        releaseQueue.Push(pool_geometry);
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

            auto img_temp = std::make_shared<BufferResource>(device, 4 * texHeight * texWidth, VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT);

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

    void createUbo()
    {
        buffer_ubo = std::make_shared<BufferResource>(device, sizeof(DataUBO), VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT);
        pool_ubo = std::make_shared<ResourcePool>();
        pool_ubo->AddResource(buffer_ubo);
        pool_ubo->Bind(device, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        pool_ubo->Map(0, sizeof(DataUBO));
        pool_ubo->SetData((void *)&dataUBO, 0, sizeof(DataUBO));

        releaseQueue.Push(buffer_ubo);
        releaseQueue.Push(pool_ubo);
    }

    void createIndirectObjects()
    {
        descriptors_bindable = std::make_shared<Descriptors>(device);
        descriptors_bindable->AddBinding(0, 0, DescriptorType::UniformBuffer, PipelineStage::Vertex, 1);
        descriptors_bindable->AddBinding(0, 1, DescriptorType::CombinedImageSampler, PipelineStage::Fragment, 4);
        descriptors_bindable->Assemble();
        descriptors_bindable->AddUpdateInfo(0, 0, {{buffer_ubo->_size, buffer_ubo}});
        descriptors_bindable->AddUpdateInfo(0, 1, {
                                                      {sampler, imgVw_bindable[0]},
                                                      {sampler, imgVw_bindable[1]},
                                                      {sampler, imgVw_bindable[2]},
                                                      {sampler, imgVw_bindable[3]},
                                                  });
        descriptors_bindable->Update();
        std::cout << "descriptors bindable updated" << std::endl;
        layout_bindable = std::make_shared<PipelineLayout>(device, descriptors_bindable);
        layout_bindable->AddPushConstant<int>(VK_SHADER_STAGE_FRAGMENT_BIT); // int index;
        layout_bindable->Assemble();

        indirectCommandsLayout = std::make_shared<IndirectCommandsLayout>(device, layout_bindable);
        indirectCommandsLayout->AddExecutionSetToken(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 2); // bound to 2 shaders
        indirectCommandsLayout->AddPushConstantToken(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(uint32_t));
        indirectCommandsLayout->AddVertexBufferToken(0);
        indirectCommandsLayout->AddVertexBufferToken(1);
        indirectCommandsLayout->AddIndexBufferToken();
        indirectCommandsLayout->AddDrawIndexedToken();
        indirectCommandsLayout->Assemble(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, false);
        std::cout << "indirect commands layout created" << std::endl;

        // melocotón pastel, amarillo pastel, rosa suave pastel, salmón claro pastel
        const std::vector<glm::vec3> shadersConst = {{0.98, 0.75, 0.65}, {0.95, 0.82, 0.60}, {0.99, 0.70, 0.70}, {0.92, 0.68, 0.55}};

        auto vertShader = std::make_shared<Shader>(device, layout_bindable);
        auto codeV = compiler.Compile(vertexShaderText, "vert.glsl", PipelineStage::Vertex, true);
        vertShader->Assemble(VK_SHADER_STAGE_VERTEX_BIT, "main", codeV, VK_SHADER_STAGE_FRAGMENT_BIT);
        shaders_bindable.push_back(vertShader);

        for (int i = 0; i < 4; i++)
        {
            auto fragShader = std::make_shared<Shader>(device, layout_bindable);
            auto code = compiler.Compile(fragShaderText, "frag_" + std::to_string(i), PipelineStage::Fragment, true);
            fragShader->AddConstant(0, sizeof(double), (void *)&shadersConst[i].r);
            fragShader->AddConstant(1, sizeof(double), (void *)&shadersConst[i].g);
            fragShader->AddConstant(2, sizeof(double), (void *)&shadersConst[i].b);
            fragShader->Assemble(VK_SHADER_STAGE_FRAGMENT_BIT, "main", code);
            shaders_bindable.push_back(fragShader);
        }

        indirectExecutionSet = std::make_shared<IndirectExecutionSet>(device, layout_bindable);
        indirectExecutionSet->Assemble({shaders_bindable[0], shaders_bindable[1]}, 5);
        indirectExecutionSet->Update(shaders_bindable);

        std::cout << "indirect execution set created & updated" << std::endl;

        auto reqs = indirectExecutionSet->GetRequirements(indirectCommandsLayout, sequences, draws);

        buffer_preprocessCommands = std::make_shared<BufferResource>(device, reqs.size, VK_BUFFER_USAGE_2_PREPROCESS_BUFFER_BIT_EXT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT);
        pool_preprocessCommands = std::make_shared<ResourcePool>();
        pool_preprocessCommands->AddResource(buffer_preprocessCommands);
        pool_preprocessCommands->Bind(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, reqs.memoryTypeBits);
        address_preprocessCommands = buffer_preprocessCommands->GetAddress();

        buffer_generatedCommands = std::make_shared<BufferResource>(device, sequences * indirectCommandsLayout->GetSize(), VK_BUFFER_USAGE_2_TRANSFER_DST_BIT | VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT);
        pool_generatedCommands = std::make_shared<ResourcePool>();
        pool_generatedCommands->AddResource(buffer_generatedCommands);
        pool_generatedCommands->Bind(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);
        address_generatedCommands = buffer_generatedCommands->GetAddress();

        releaseQueue.Push(descriptors_bindable);
        releaseQueue.Push(layout_bindable);
        releaseQueue.Push(indirectCommandsLayout);
        for (auto &shader : shaders_bindable)
        {
            releaseQueue.Push(shader);
        }
        releaseQueue.Push(indirectExecutionSet);
        releaseQueue.Push(buffer_preprocessCommands);
        releaseQueue.Push(pool_preprocessCommands);
        releaseQueue.Push(buffer_generatedCommands);
        releaseQueue.Push(pool_generatedCommands);
    }

    void createIndirectCommandsGenerator()
    {
        descriptors_genCommands = std::make_shared<Descriptors>(device);
        descriptors_genCommands->AddBinding(0, 0, DescriptorType::StorageBuffer, PipelineStage::Compute, 1); // indirect buffer
        descriptors_genCommands->Assemble();
        descriptors_genCommands->AddUpdateInfo(0, 0, {{buffer_generatedCommands->_size, buffer_generatedCommands}});
        descriptors_genCommands->Update();
        layout_genCommands = std::make_shared<PipelineLayout>(device, descriptors_genCommands);
        layout_genCommands->AddPushConstant(sizeof(uint64_t) + sizeof(int) + sizeof(int), VK_SHADER_STAGE_COMPUTE_BIT); // uint64_t address;
                                                                                                                        // int indexCount;
                                                                                                                        // int vertexCount;
        layout_genCommands->Assemble();

        auto compShader = std::make_shared<ShaderModule>(device, compiler, compShaderText, "main", "compute.glsl", PipelineStage::Compute, true);
        pipeline_genCommands = std::make_shared<ComputePipeline>(device, layout_genCommands, compShader);
        compShader->Release();

        releaseQueue.Push(descriptors_genCommands);
        releaseQueue.Push(layout_genCommands);
        releaseQueue.Push(pipeline_genCommands);
    }

    struct Sequence
    {
        uint32_t shaderIndex1;
        uint32_t shaderIndex2;
        int pushValue; // texture index
        VkBindVertexBufferIndirectCommandEXT vertex0;
        VkBindVertexBufferIndirectCommandEXT vertex1;
        VkBindIndexBufferIndirectCommandEXT index;
        VkDrawIndexedIndirectCommand draw;
    };
    void genCommandsCPU()
    {
        std::vector<Sequence> sequences;
        for (uint32_t i = 0; i < 4; i++)
        {
            for (uint32_t j = 0; j < 4; j++)
            {
                Sequence seq;
                seq.shaderIndex1 = 0; // selects first shader always
                seq.shaderIndex2 = i; // selects fragment shader

                seq.pushValue = j; // selects image

                seq.vertex0.bufferAddress = pcData_gen.address_geometry;
                seq.vertex0.size = 12; // sizeof(vec3)
                seq.vertex0.stride = 12 + 8;

                seq.vertex1.bufferAddress = pcData_gen.address_geometry;
                seq.vertex1.size = 8; // sizeof(vec2)
                seq.vertex1.stride = 12 + 8;

                seq.index.bufferAddress = pcData_gen.address_geometry + ((12 + 8) * pcData_gen.vertexCount);
                seq.index.size = 4; // sizeof(uint32)
                seq.index.indexType = VK_INDEX_TYPE_UINT32;

                seq.draw.indexCount = pcData_gen.indexCount;
                seq.draw.instanceCount = 1;
                seq.draw.firstIndex = 0;
                seq.draw.vertexOffset = 0;
                seq.draw.firstInstance = 0;

                // uint32_t index = (4 * j) + i;
                // ind.sequences[index] = seq;
                sequences.push_back(seq);
            }
        }

        auto poolTemp = std::make_shared<ResourcePool>();

        auto fullSize = buffer_generatedCommands->_size;
        auto bufferTemp = std::make_shared<BufferResource>(device, fullSize, VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT);

        poolTemp->AddResource(bufferTemp);

        poolTemp->Bind(device, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        poolTemp->Map(0, fullSize);
        poolTemp->SetData((void *)sequences.data(), 0, fullSize);
        poolTemp->UnMap();

        manager->Prepare();
        manager->AddTransference(bufferTemp, buffer_generatedCommands, TransferType::HostToDevice);
        manager->TransferAll();

        bufferTemp->Release();
        poolTemp->Release();
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
        createUbo();
        createIndirectObjects();
        createIndirectCommandsGenerator();
        //genCommandsCPU();

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
        checkResult(vkCreateCommandPool(device->GetHandle(), &poolCI, nullptr, &pool));

        VkCommandBufferAllocateInfo cmdAI = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        checkResult(vkAllocateCommandBuffers(device->GetHandle(), &cmdAI, &cmd));
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

    void recordShaderRenderingData(VkCommandBuffer cmd)
    {
        VkVertexInputAttributeDescription2EXT attributes[2];
        attributes[0] = {
            .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = 0,
        };
        attributes[1] = {
            .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
            .location = 1,
            .binding = 0,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = sizeof(glm::vec3),
        };

        VkVertexInputBindingDescription2EXT binding = {
            .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
            .binding = 0,
            .stride = sizeof(glm::vec3) + sizeof(glm::vec2),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            .divisor = 1,
        };

        vkCmdSetVertexInputEXT(cmd, 1, &binding, 2, attributes);

        vkCmdSetLineWidth(cmd, 1.0f);

        VkViewport viewport = {0, 0, (float)width, (float)height, 0.0f, 1.0f};
        vkCmdSetViewportWithCountEXT(cmd, 1, &viewport);

        VkRect2D scissor = {{0, 0}, {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}};
        vkCmdSetScissorWithCountEXT(cmd, 1, &scissor);

        vkCmdSetPrimitiveTopologyEXT(cmd, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

        vkCmdSetFrontFaceEXT(cmd, VK_FRONT_FACE_COUNTER_CLOCKWISE);

        VkColorComponentFlags colorFlags = VK_COMPONENT_SWIZZLE_R | VK_COMPONENT_SWIZZLE_G | VK_COMPONENT_SWIZZLE_B | VK_COMPONENT_SWIZZLE_A;
        vkCmdSetColorWriteMaskEXT(cmd, 0, 1, &colorFlags);

        vkCmdSetPrimitiveRestartEnableEXT(cmd, VK_FALSE);

        vkCmdSetCullModeEXT(cmd, VK_CULL_MODE_BACK_BIT);

        vkCmdSetRasterizationSamplesEXT(cmd, VK_SAMPLE_COUNT_1_BIT);
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

        // gen commands
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, layout_genCommands->GetHandle(), 0,
                                (uint32_t)descriptors_genCommands->_descriptorSets.size(),
                                descriptors_genCommands->_descriptorSets.data(), 0, nullptr);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_genCommands->GetHandle());

        uint32_t pcFullSize = sizeof(uint64_t) + sizeof(int) + sizeof(int);
        vkCmdPushConstants(cmd, layout_genCommands->GetHandle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, pcFullSize, (void *)&pcData_gen);
        vkCmdDispatch(cmd, 1, 1, 1); // workgroup (4,4,0)

        // VkMemoryBarrier2 memBarrier1 = {
        //     .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
        //     .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
        //     .dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
        // };

        VkBufferMemoryBarrier2 bufferBarrier = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            .srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_COMMAND_PREPROCESS_BIT_EXT,
            .dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = buffer_generatedCommands->GetHandle(),
            .offset = 0,
            .size = buffer_generatedCommands->_size,
        };

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
            .bufferMemoryBarrierCount = 1,
            .pBufferMemoryBarriers = &bufferBarrier,
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

        // execute commands

        VkGeneratedCommandsInfoEXT genCommandsI = {
            .sType = VK_STRUCTURE_TYPE_GENERATED_COMMANDS_INFO_EXT,
            .shaderStages = indirectCommandsLayout->_shaderStages,
            .indirectExecutionSet = indirectExecutionSet->GetHandle(),
            .indirectCommandsLayout = indirectCommandsLayout->_commandsLayout,
            .indirectAddress = (VkDeviceAddress)address_generatedCommands,
            .indirectAddressSize = (VkDeviceSize)buffer_generatedCommands->_size,
            .preprocessAddress = (VkDeviceAddress)address_preprocessCommands,
            .preprocessSize = (VkDeviceSize)buffer_preprocessCommands->_size,
            .maxSequenceCount = 16,
            // .sequenceCountAddress;
            .maxDrawCount = 16,
        };
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout_bindable->GetHandle(), 0,
                                (uint32_t)descriptors_bindable->_descriptorSets.size(),
                                descriptors_bindable->_descriptorSets.data(), 0, nullptr);

        recordShaderRenderingData(cmd);

        VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
        VkShaderEXT shaders[] = {shaders_bindable[0]->_shader, shaders_bindable[1]->_shader};

        vkCmdBindShadersEXT(cmd, 2, stages, shaders);

        vkCmdExecuteGeneratedCommandsEXT(cmd, VK_FALSE, &genCommandsI);

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

    void beforeResize()
    {
        camera = Lettuce::X3D::Camera3D::Camera3D(width, height);
        camera.Reset(glm::vec3(20, 20, 30), glm::vec3(0.0f), glm::vec3(0.57735026919)); // 1 / sqrt(3)
    }

    void destroyObjects()
    {
        vkFreeCommandBuffers(device->GetHandle(), pool, 1, &cmd);
        vkDestroyCommandPool(device->GetHandle(), pool, nullptr);

        pool_ubo->UnMap();
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