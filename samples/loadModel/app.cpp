#include "Lettuce/Lettuce.hpp"
#include "glfw/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw/glfw3native.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <windows.h>

#include <memory>
#include <vector>
#include <expected>
#include <thread>
#include <chrono>
#include <print>
#include <fstream>
#include <filesystem>
#include <source_location>
#include <optional>
#include <functional>

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include <meshoptimizer.h>

using namespace Lettuce::Core;
using namespace Lettuce::Rendering;

struct SceneData
{
    float4x4 viewProj;
};

struct Instance {
    uint32_t meshID;
    float4x4 model;
};

struct MeshInfo {
    uint32_t primitiveOffset;
    uint32_t primitiveCount;
};

struct PrimitiveInfo {
    uint32_t vertexOffset;
    uint32_t vertexCount;
    uint32_t indexOffset;
    uint32_t indexCount;
};

struct Vertex {
    float3 position;
    float3 normal;
    float3 tangent;
    float2 texCoord0;
};

struct InstancedPrimitive
{
    uint32_t primitiveID, instanceID;
};

GLFWwindow* window;

constexpr uint32_t width = 1366;
constexpr uint32_t height = 768;

Device device;
Swapchain swapchain;
DescriptorTable descriptorTable;
Pipeline cullPipeline;
Pipeline rgbPipeline;
CommandAllocator cmdAlloc;

MemoryView mvSceneData;
MemoryView mvIndexB, mvVertexB, mvInstances, mvMeshes, mvPrimitives, mvInstancedPrimitives;
MemoryView mvIndirectB;
MemoryView mvDebugBuffer, mvPickInstanceData;

MemoryViewInfo mviSceneData;
MemoryViewInfo mviIndexB, mviVertexB, mviInstances, mviMeshes, mviPrimitives, mviInstancedPrimitives;
MemoryViewInfo mviIndirectB;
MemoryViewInfo mviDebugBuffer, mviPickInstanceData;

IndirectSet isIndirect;

TextureView tDepthTarget;
TextureView tPickTexture;

std::vector<MeshInfo> meshes;
std::vector<PrimitiveInfo> primitives;

constexpr uint32_t debugBufferCount = 32;
constexpr uint32_t debugBufferItemSize = 4 * sizeof(uint32_t);

Lettuce::Utils::FrameTimer timer;
Lettuce::Utils::Camera3DDesc camera2Desc;
Lettuce::Utils::Camera3D camera2(camera2Desc); // explicit constructor

double xprev = width / 2;
double yprev = height / 2;
bool wasMousePressed = false;

void UpdateCamera2()
{
    double dt = timer.GetDeltaTime();

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    bool mousePressed = GLFW_PRESS == glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

    bool aKeyPressed = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_A) || GLFW_PRESS == glfwGetKey(window, GLFW_KEY_LEFT);
    bool wKeyPressed = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_W) || GLFW_PRESS == glfwGetKey(window, GLFW_KEY_UP);
    bool sKeyPressed = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_S) || GLFW_PRESS == glfwGetKey(window, GLFW_KEY_DOWN);
    bool dKeyPressed = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_D) || GLFW_PRESS == glfwGetKey(window, GLFW_KEY_RIGHT);

    if (mousePressed && !wasMousePressed)
    {
        xprev = xpos;
        yprev = ypos;
    }

    if (mousePressed)
    {
        camera2.Rotate({ static_cast<float>(xpos - xprev), static_cast<float>(ypos - yprev) });
    }

    auto scenePtr = ((SceneData*)(mviSceneData.cpuAddress));
    scenePtr->viewProj = camera2.Update({ wKeyPressed,aKeyPressed,sKeyPressed, dKeyPressed,static_cast<float>(dt) });

    xprev = xpos;
    yprev = ypos;
    wasMousePressed = mousePressed;
}

void initLettuce()
{
    auto hwnd = glfwGetWin32Window(window);
    auto hmodule = GetModuleHandle(NULL);

    DeviceDesc deviceCI = {
        .preferDedicated = true,
    };
    device.Create(deviceCI);

    SwapchainDesc swapchainDesc = {
        .width = width,
        .height = height,
        .clipped = true,
        .windowPtr = &hwnd,
        .applicationPtr = &hmodule,
    };
    swapchain = device.CreateSwapchain(swapchainDesc);

    CommandAllocatorDesc cmdAllocDesc = {
        .queueType = QueueType::Graphics,
    };
    cmdAlloc = device.CreateCommandAllocator(cmdAllocDesc);

    mvSceneData = device.CreateMemoryView({ sizeof(SceneData), true });
    mviSceneData = device.GetMemoryViewInfo(mvSceneData);

    mvPickInstanceData = device.CreateMemoryView({ sizeof(uint32_t), true });
    mviPickInstanceData = device.GetMemoryViewInfo(mvPickInstanceData);

    TextureViewDesc pickDesc = {
        .width = width,
        .height = height,
        .depth = 1,
        .format = Format::Raw_R32_UInt,
        .mipCount = 1,
        .layerCount = 1,
        .cpuVisible = false,
    };
    tPickTexture = device.CreateTextureView(pickDesc);

    IndirectSetDesc isDesc =
    {
        .type = IndirectType::Draw,
        .maxCount = 1024,
        .userDataSize = 0,
    };
    isIndirect = device.CreateIndirectSet(isDesc);
    mvIndirectB = device.GetIndirectSetView(isIndirect);
    mviIndirectB = device.GetMemoryViewInfo(mvIndirectB);

    // DEBUG BUFFER, CPU READEABLE
    mvDebugBuffer = device.CreateMemoryView({ debugBufferCount * debugBufferItemSize, true });
    mviDebugBuffer = device.GetMemoryViewInfo(mvDebugBuffer);

    RenderTargetDesc depthDesc = {
        .width = width,
        .height = height,
        .type = RenderTargetType::Depth_D32,
        .defaultClearValue = DepthStencilClear {1.0f, 0},
    };
    tDepthTarget = device.CreateTextureView(depthDesc);
}

void createRenderingObjects()
{
    auto shaders = Lettuce::Utils::AssetLoader::LoadSpirv(&device, "samples/loadModel/loadModel.spv");

    DescriptorTableDesc descriptorTableDesc = { 4,4,4 };
    descriptorTable = device.CreateDescriptorTable(descriptorTableDesc);

    std::array<std::pair<uint32_t, TextureView>, 1> texDescs;
    texDescs[0] = { 0, tPickTexture };

    PushResourceDescriptorsDesc pushDtDesc = {
        .storageTextures = std::span(texDescs),
        .descriptorTable = descriptorTable,
    };
    device.PushResourceDescriptors(pushDtDesc);

    std::array<Format, 1> formatArr = { device.GetRenderTargetFormat(swapchain) };
    PrimitiveShadingPipelineDesc pipelineDesc = {
        .fragmentShadingRate = false,
        .vertEntryPoint = "vertexMain",
        .fragEntryPoint = "fragmentMain",
        .vertShaderBinary = shaders,
        .fragShaderBinary = shaders,
        .colorAttachmentFormats = std::span(formatArr),
        .depthStencilAttachmentFormat = Format::Universal_DepthStencil_D32_SFloat_S8_UInt,
        .descriptorTable = descriptorTable,
    };
    rgbPipeline = device.CreatePipeline(pipelineDesc);

    ComputePipelineDesc compPipelineDesc = {
        .compEntryPoint = "cullMain",
        .compShaderBinary = shaders,
        .descriptorTable = descriptorTable,
    };
    cullPipeline = device.CreatePipeline(compPipelineDesc);

    device.Destroy(shaders);
}

void loadModel()
{
    std::filesystem::path modelPath = "../../../../external/models/DragonAttenuation.glb";

    if (!std::filesystem::exists(modelPath))
    {
        std::println("file {} does not exist", "DragonAttenuation.glb");
        return;
    }

    auto parser = fastgltf::Parser();
    auto gltfData = fastgltf::GltfDataBuffer::FromPath(modelPath);

    auto asset = parser.loadGltf(gltfData.get(), modelPath.parent_path(), fastgltf::Options::None);

    meshes = std::vector<MeshInfo>();
    primitives = std::vector<PrimitiveInfo>();
    auto vertexVec = std::vector<Vertex>();
    auto indexVec = std::vector<uint32_t>();

    int meshCount = 0;
    int totalPrims = 0;
    for (auto& mesh : asset->meshes)
    {
        int primCount = 0;
        for (auto& prim : mesh.primitives)
        {
            auto* posIt = prim.findAttribute("POSITION");
            auto* norIt = prim.findAttribute("NORMAL");
            auto* tanIt = prim.findAttribute("TANGENT");
            auto* uvsIt = prim.findAttribute("TEXCOORD_0");

            auto& posAcc = asset->accessors[posIt->accessorIndex];
            auto& norAcc = asset->accessors[norIt->accessorIndex];
            auto& tanAcc = asset->accessors[tanIt->accessorIndex];
            auto& uvsAcc = asset->accessors[uvsIt->accessorIndex];
            auto& idxAcc = asset->accessors[prim.indicesAccessor.value()];

            auto baseVertexIdx = vertexVec.size();
            auto baseIndexIdx = indexVec.size();

            vertexVec.resize(baseVertexIdx + posAcc.count);
            indexVec.resize(baseIndexIdx + idxAcc.count);

            PrimitiveInfo prim = { baseVertexIdx, posAcc.count, baseIndexIdx, idxAcc.count };
            primitives.push_back(prim);

            fastgltf::iterateAccessorWithIndex<float3>(asset.get(), posAcc, [&](float3 pos, size_t idx) {
                vertexVec[baseVertexIdx + idx].position = pos;
                });
            std::println("pos acc");
            fastgltf::iterateAccessorWithIndex<float3>(asset.get(), norAcc, [&](float3 normal, size_t idx) {
                vertexVec[baseVertexIdx + idx].normal = normal;
                });
            std::println("nor acc");

            fastgltf::iterateAccessorWithIndex<float3>(asset.get(), tanAcc, [&](float3 tang, size_t idx) {
                vertexVec[baseVertexIdx + idx].tangent = tang;
                });
            std::println("tan acc");

            fastgltf::iterateAccessorWithIndex<float2>(asset.get(), uvsAcc, [&](float2 uv, size_t idx) {
                vertexVec[baseVertexIdx + idx].texCoord0 = uv;
                });
            std::println("tex0 acc");

            fastgltf::iterateAccessorWithIndex<uint32_t>(asset.get(), idxAcc, [&](uint32_t index, size_t idx) {
                indexVec[baseIndexIdx + idx] = index;
                });
            std::println("idx acc");

            std::println("mesh #{}, primitive  #{} : #vertex:{}  #index: {}", meshCount, primCount, posAcc.count, idxAcc.count);
            ++primCount;
        }
        MeshInfo mesh;
        mesh.primitiveOffset = totalPrims;
        mesh.primitiveCount = primCount;
        meshes.push_back(mesh);
        totalPrims += primCount;
        ++meshCount;
    }

    mvMeshes = device.CreateMemoryView({ sizeof(MeshInfo) * meshes.size(), true });
    mvPrimitives = device.CreateMemoryView({ sizeof(PrimitiveInfo) * primitives.size(), true });
    mvVertexB = device.CreateMemoryView({ sizeof(Vertex) * vertexVec.size(), true });
    mvIndexB = device.CreateMemoryView({ sizeof(uint32_t) * indexVec.size(), true });

    mviMeshes = device.GetMemoryViewInfo(mvMeshes);
    mviPrimitives = device.GetMemoryViewInfo(mvPrimitives);
    mviVertexB = device.GetMemoryViewInfo(mvVertexB);
    mviIndexB = device.GetMemoryViewInfo(mvIndexB);

    memcpy(mviMeshes.cpuAddress, meshes.data(), sizeof(MeshInfo) * meshes.size());
    memcpy(mviPrimitives.cpuAddress, primitives.data(), sizeof(PrimitiveInfo) * primitives.size());
    memcpy(mviVertexB.cpuAddress, vertexVec.data(), sizeof(Vertex) * vertexVec.size());
    memcpy(mviIndexB.cpuAddress, indexVec.data(), sizeof(uint32_t) * indexVec.size());
}

uint32_t instanceCount = 0;
uint32_t instancedPrimitivesCount = 0;
void loadInstances()
{
    std::vector<Instance> instances;
    instances.reserve(20);

    const int gridX = 5;
    const int gridY = 4;
    const float spacing = 8.0f;

    for (int y = 0; y < gridY; y++)
    {
        for (int x = 0; x < gridX; x++)
        {
            int i = y * gridX + x;

            glm::vec3 pos = {
                (x - gridX / 2) * spacing,
                0.0f,
                (y - gridY / 2) * spacing
            };

            float angle = glm::radians(20.0f * i);

            float uniformScale = 0.5f;
            glm::mat4 model =
                glm::translate(glm::mat4(1.0f), pos) *
                glm::rotate(glm::mat4(1.0f), angle, { 0,1,0 }) *
                glm::scale(glm::mat4(1.0f), { uniformScale,uniformScale,uniformScale });

            Instance inst;
            inst.meshID = (i < 10) ? 0 : 1;
            inst.model = model;

            instancedPrimitivesCount += meshes[inst.meshID].primitiveCount;

            instances.push_back(inst);
        }
    }

    mvInstances = device.CreateMemoryView({ sizeof(uint32_t) + (sizeof(Instance) * instances.size()), true });
    mvInstancedPrimitives = device.CreateMemoryView({ sizeof(InstancedPrimitive) * instancedPrimitivesCount, true });

    mviInstances = device.GetMemoryViewInfo(mvInstances);
    mviInstancedPrimitives = device.GetMemoryViewInfo(mvInstancedPrimitives);

    // instances Buffer layout: [ count | instances ]
    instanceCount = instances.size();
    *(uint32_t*)(mviInstances.cpuAddress) = instances.size();
    memcpy(sizeof(uint32_t) + (uint8_t*)(mviInstances.cpuAddress), instances.data(), sizeof(Instance) * instances.size());
}

void mainLoop()
{
    timer.Start();

    while (!glfwWindowShouldClose(window))
    {
        timer.Tick();
        UpdateCamera2();

        // not optimal, but works
        double xCursorPos, yCursorPos;
        bool isPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        glfwGetCursorPos(window, &xCursorPos, &yCursorPos);
        int mode = glfwGetInputMode(window, GLFW_CURSOR);
        if (mode == GLFW_CURSOR_DISABLED)
        {
            xCursorPos = width / 2;
            yCursorPos = height / 2;
        }
        // std::println("cursor at x: {} y: {}", xCursorPos, yCursorPos);

        device.NextFrame(swapchain);

        device.Reset(cmdAlloc);
        auto frame = device.GetCurrentRenderTarget(swapchain);
        auto cmd = device.AllocateCommandBuffer(cmdAlloc);

        AttachmentDesc colorAttachment[1] = {
            {
                .renderTarget = frame,
                .loadOp = LoadOp::Clear,
            }
        };
        AttachmentDesc depthAttachment = {
            .renderTarget = tDepthTarget,
            .loadOp = LoadOp::Clear,
        };

        RenderPassDesc renderPassDesc = {
            .width = width,
            .height = height,
            .colorAttachments = std::span(colorAttachment),
            .depthStencilAttachment = depthAttachment,
        };

        auto allocs = std::array{
            mvSceneData,
            mvInstances,
            mvMeshes,
            mvPrimitives,
            mvVertexB,
            mvIndexB,
            mvIndirectB,
            mvInstancedPrimitives,
            mvPickInstanceData,
            mvDebugBuffer,
        };

        PushAllocationsDesc pushDesc = {
            .allocations = allocs,
            .descriptorTable = descriptorTable,
        };

        ExecuteIndirectDesc execDesc = {
            .indirectSet = isIndirect,
            .maxDrawCount = instancedPrimitivesCount,
        };

        TextureToMemory tmPixelCopy =
        {
            .srcTexture = tPickTexture,
            .dstMemory = mvPickInstanceData,
            .mipmapLevel = 0,
            .layerBaseLevel = 0,
            .layerCount = 1,
            .x = static_cast<uint32_t>(xCursorPos), .y = static_cast<uint32_t>(yCursorPos), .width = 1, .height = 1,
        };

        ClearTextureDesc clearDesc = {
            .texture = tPickTexture,
            .color = {1.0f},
            .baseLevel = 0,
            .levelCount = 1,
            .baseLayer = 0,
            .layerCount = 1,
        };

        BarrierDesc bCopyComp[] = { {
            .srcAccess = PipelineAccess::Write,
            .srcStage = PipelineStage::ComputeShader,
            .dstAccess = PipelineAccess::Read,
            .dstStage = PipelineStage::ComputeShader,
        }, };

        BarrierDesc bCompInd[] = { {
            .srcAccess = PipelineAccess::Write,
            .srcStage = PipelineStage::ComputeShader,
            .dstAccess = PipelineAccess::Read,
            .dstStage = PipelineStage::DrawIndirect,
        }, };

        BarrierDesc bFragCopy[] = { {
            .srcAccess = PipelineAccess::Write,
            .srcStage = PipelineStage::FragmentShader,
            .dstAccess = PipelineAccess::Read,
            .dstStage = PipelineStage::Copy,
        }, };

        cmd.ResetCount(isIndirect);
        cmd.ClearTexture(clearDesc);

        cmd.Barrier(bCopyComp);

        cmd.BindDescriptorTable(descriptorTable, PipelineBindPoint::Compute);
        cmd.BindPipeline(cullPipeline);
        cmd.PushAllocations(pushDesc);
        auto dispatchX = (instanceCount + 31) / 32;
        cmd.Dispatch(dispatchX, 1, 1);

        cmd.Barrier(bCompInd);

        cmd.BeginRendering(renderPassDesc);
        cmd.BindDescriptorTable(descriptorTable, PipelineBindPoint::Graphics);
        cmd.BindPipeline(rgbPipeline);
        cmd.PushAllocations(pushDesc);
        cmd.ExecuteIndirect(execDesc);
        cmd.EndRendering();
        if (isPressed)
        {
            cmd.Barrier(bFragCopy);
            cmd.MemoryCopy(tmPixelCopy);
        }
        else
        {
            *((uint32_t*)mviPickInstanceData.cpuAddress) = 0;
        }

        std::array<std::span<CommandBuffer>, 1> cmds = { std::span(&cmd, 1) };

        CommandBufferSubmitDesc submitDesc = {
            .queueType = QueueType::Graphics,
            .commandBuffers = std::span(cmds),
            .presentSwapchain = swapchain,
        };

        device.Submit(submitDesc);

        device.DisplayFrame(swapchain);
        device.WaitFor(QueueType::Graphics);
        if (isPressed)
            std::println("picked instance: {}", *((uint32_t*)mviPickInstanceData.cpuAddress) - 1);
        // auto baseGenDrawCallPtr = (VkDrawIndirectCommand*)mviDebugBuffer.cpuAddress;
        // for (size_t i = 0; i < debugBufferCount; ++i) {
        //     auto genDrawCallPtr = (baseGenDrawCallPtr + i);
        //     std::print(" {},{},{},{} |", genDrawCallPtr->vertexCount, genDrawCallPtr->instanceCount, genDrawCallPtr->firstVertex, genDrawCallPtr->firstVertex);
        // }
        // std::println();

        glfwPollEvents();
    }
}

void cleanupLettuce()
{
    device.WaitFor(QueueType::Graphics);
    device.Destroy(rgbPipeline);
    device.Destroy(cullPipeline);
    device.Destroy(descriptorTable);

    std::vector<MemoryView> destroyableMemoryViews = {
        mvSceneData,
        mvIndexB, mvVertexB, mvInstances, mvMeshes, mvPrimitives, mvInstancedPrimitives,
        mvDebugBuffer, mvPickInstanceData,
    };
    for (auto mv : destroyableMemoryViews)
        device.Destroy(mv);

    device.Destroy(tDepthTarget);
    device.Destroy(tPickTexture);

    device.Destroy(isIndirect);
    device.Destroy(cmdAlloc);
    device.Destroy(swapchain);
    device.Destroy();
}

void initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(width, height, "My Lettuce Window", NULL, NULL);
}

void cleanupWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

int main()
{
    std::ios::sync_with_stdio(true);
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    initWindow();
    initLettuce();
    createRenderingObjects();
    loadModel();
    loadInstances();
    mainLoop();
    cleanupLettuce();
    cleanupWindow();
    return 0;
}