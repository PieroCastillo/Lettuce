#include "Lettuce/Lettuce.hpp"
#include "glfw/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw/glfw3native.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <windows.h>

#include <array>
#include <chrono>
#include <expected>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <optional>
#include <print>
#include <thread>
#include <vector>

using namespace Lettuce::Core;
using namespace Lettuce::Rendering;
using namespace Lettuce::Utils;

struct SceneData
{
    float4x4 viewProj;
};

GLFWwindow* window;

constexpr uint32_t width = 1366;
constexpr uint32_t height = 768;
double xprev = width / 2;
double yprev = height / 2;
bool wasMousePressed = false;

std::unique_ptr<Device> device;
std::unique_ptr<SceneTree> sceneTree;

Swapchain swapchain;
CommandAllocator cmdAlloc;
IndirectSet indirectSet;
DescriptorTable dtScene;
Pipeline pDrawScene;
uint32_t drawCallCount = 0;

TextureView tDepthTarget;
MemoryView mvSceneData;
MemoryView mvClusterBuildCmds;
MemoryView mvTriangleBuffer;
MemoryView mvVertexBuffer;
MemoryView mvIndirectCalls;
MemoryViewInfo mviSceneData;
MemoryViewInfo mviClusterBuildCmds;
MemoryViewInfo mviTriangleBuffer;
MemoryViewInfo mviVertexBuffer;

FrameTimer timer;
Camera3D camera(Camera3DDesc{});

void UpdateCamera()
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
        camera.Rotate({ static_cast<float>(xpos - xprev), static_cast<float>(ypos - yprev) });
    }

    auto scenePtr = ((SceneData*)(mviSceneData.cpuAddress));
    scenePtr->viewProj = camera.Update({ wKeyPressed,aKeyPressed,sKeyPressed, dKeyPressed,static_cast<float>(dt) });

    xprev = xpos;
    yprev = ypos;
    wasMousePressed = mousePressed;
}

void initLettuce()
{
    auto hwnd = glfwGetWin32Window(window);
    auto hmodule = GetModuleHandle(NULL);

    device = std::make_unique<Device>();

    DeviceDesc deviceCI = {
        .preferDedicated = true,
    };
    device->Create(deviceCI);

    SwapchainDesc swapchainDesc = {
        .width = width,
        .height = height,
        .clipped = true,
        .windowPtr = &hwnd,
        .applicationPtr = &hmodule,
    };
    swapchain = device->CreateSwapchain(swapchainDesc);

    CommandAllocatorDesc cmdAllocDesc = {
        .queueType = QueueType::Graphics,
    };
    cmdAlloc = device->CreateCommandAllocator(cmdAllocDesc);
}

std::vector<VkDrawIndirectCommand> indirectCalls;
void createResources()
{
    tDepthTarget = device->CreateTextureView({ width, height, RenderTargetType::Depth_D32, DepthStencilClear {1.0f, 0} });

    mvSceneData = device->CreateMemoryView({ sizeof(SceneData), true });
    mviSceneData = device->GetMemoryViewInfo(mvSceneData);

    auto modelData = Lettuce::Utils::AssetLoader::LoadGltfModel(device.get(), "../../../../external/models/DragonAttenuation.glb");
    std::println("model loaded successfully!");

    indirectSet = device->CreateIndirectSet({ IndirectType::Draw, (uint32_t)modelData.clusterBuilds.size(), 0 });

    /* create memory resources*/
    mvClusterBuildCmds = device->CreateMemoryView({ sizeof(ClusterBuildIndirectCommand) * modelData.clusterBuilds.size() ,true });
    mvTriangleBuffer = device->CreateMemoryView({ sizeof(uint8_t) * modelData.trianglesTable.size(), true });
    mvVertexBuffer = device->CreateMemoryView({ sizeof(VertexL) * modelData.vertexTable.size(), true });
    mvIndirectCalls = device->GetIndirectSetView(indirectSet);

    /* create indirect calls*/
    auto mesh = modelData.meshes[0];
    drawCallCount = mesh.clusterCount; // SET DRAW COUNT 
    // std::vector<VkDrawIndirectCommand> indirectCalls;
    indirectCalls.reserve(mesh.clusterCount);
    for (uint32_t i = mesh.clusterOffset; i < mesh.clusterOffset + mesh.clusterCount; ++i)
    {
        indirectCalls.push_back({ modelData.clusterBuilds[i].triangleCount * 3, 1,0,0 });
    }

    /* prepare staging buffer*/
    auto mvTempIndirectCalls = device->CreateMemoryView({ sizeof(VkDrawIndirectCommand) * indirectCalls.size(), true });

    mviClusterBuildCmds = device->GetMemoryViewInfo(mvClusterBuildCmds);
    mviTriangleBuffer = device->GetMemoryViewInfo(mvTriangleBuffer);
    mviVertexBuffer = device->GetMemoryViewInfo(mvVertexBuffer);
    auto mviTempIndirectCalls = device->GetMemoryViewInfo(mvTempIndirectCalls);

    memcpy(mviClusterBuildCmds.cpuAddress, &modelData.clusterBuilds[0], sizeof(ClusterBuildIndirectCommand) * modelData.clusterBuilds.size());
    memcpy(mviTriangleBuffer.cpuAddress, &modelData.trianglesTable[0], sizeof(uint8_t) * modelData.trianglesTable.size());
    memcpy(mviVertexBuffer.cpuAddress, &modelData.vertexTable[0], sizeof(VertexL) * modelData.vertexTable.size());
    memcpy(mviTempIndirectCalls.cpuAddress, &indirectCalls[0], sizeof(VkDrawIndirectCommand) * drawCallCount);

    // perform commands copy
    auto cmd = device->AllocateCommandBuffer(cmdAlloc);
    cmd.Fill(mvIndirectCalls, 0, drawCallCount, 1);
    cmd.MemoryCopy({
        mvTempIndirectCalls, mvIndirectCalls,
        sizeof(VkDrawIndirectCommand) * drawCallCount, 0, 4
        });

    std::array<std::span<CommandBuffer>, 1> cmds = { std::span(&cmd, 1) };

    CommandBufferSubmitDesc submitDesc = {
        .queueType = QueueType::Graphics,
        .commandBuffers = std::span(cmds),
    };

    device->Submit(submitDesc);
    device->WaitFor(QueueType::Graphics);
    device->Destroy(mvTempIndirectCalls); // destroy temp buffer

    // access test
    auto scneMat = ((SceneData*)mviSceneData.cpuAddress)[0].viewProj;
    for (uint32_t drawID = 0; drawID < indirectCalls.size(); ++drawID)
    {
        auto& drawCall = indirectCalls[drawID];
        for (uint32_t vertexID = 0; vertexID < drawCall.vertexCount; ++vertexID)
        {
            // check access
            auto cmd = ((ClusterBuildIndirectCommand*)mviClusterBuildCmds.cpuAddress)[drawID];
            auto idx = ((uint8_t*)mviTriangleBuffer.cpuAddress)[cmd.triangleOffset + vertexID];
            auto vertex = ((VertexL*)mviVertexBuffer.cpuAddress)[cmd.vertexOffset + idx];

            std::println("cluster {}, local index: {}, vertex pos {}{}{}", cmd.clusterID, idx, vertex.pos.x,vertex.pos.y, vertex.pos.z);
        }
    }
}

void createRenderingObjects()
{
    auto shader = AssetLoader::LoadSpirv(device.get(), "samples/sceneTreeDemo/sceneTreeDemo.spv");

    dtScene = device->CreateDescriptorTable({ 4,4,4 });

    std::array<Format, 1> formatArr = { device->GetRenderTargetFormat(swapchain) };
    pDrawScene = device->CreatePipeline({
        .fragmentShadingRate = false,
        .vertEntryPoint = "vertMain",
        .fragEntryPoint = "fragMain",
        .vertShaderBinary = shader,
        .fragShaderBinary = shader,
        .colorAttachmentFormats = std::span(formatArr),
        .depthStencilAttachmentFormat = Format::Universal_Depth_D32_SFloat,
        .descriptorTable = dtScene,
        });

    device->Destroy(shader);
}

void drawScene(CommandBuffer& cmd)
{
    auto allocs = std::array{
        mvSceneData,
        mvClusterBuildCmds,
        mvTriangleBuffer,
        mvVertexBuffer,
    };

    cmd.BindDescriptorTable(dtScene, PipelineBindPoint::Graphics);
    cmd.BindPipeline(pDrawScene);
    cmd.PushAllocations({ allocs, dtScene });
    cmd.ExecuteIndirect({ indirectSet, drawCallCount });
}

void mainLoop()
{
    timer.Start();

    while (!glfwWindowShouldClose(window))
    {
        timer.Tick();
        UpdateCamera();

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

        device->NextFrame(swapchain);

        device->Reset(cmdAlloc);
        auto frame = device->GetCurrentRenderTarget(swapchain);
        auto cmd = device->AllocateCommandBuffer(cmdAlloc);

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

        // commands go here
        cmd.BeginRendering(renderPassDesc);
        drawScene(cmd);
        cmd.EndRendering();

        std::array<std::span<CommandBuffer>, 1> cmds = { std::span(&cmd, 1) };

        CommandBufferSubmitDesc submitDesc = {
            .queueType = QueueType::Graphics,
            .commandBuffers = std::span(cmds),
            .presentSwapchain = swapchain,
        };

        device->Submit(submitDesc);
        device->DisplayFrame(swapchain);
        device->WaitFor(QueueType::Graphics);

        glfwPollEvents();
    }
}

void cleanupLettuce()
{
    device->WaitFor(QueueType::Graphics);

    device->Destroy(pDrawScene);
    device->Destroy(dtScene);

    device->Destroy(mvVertexBuffer);
    device->Destroy(mvTriangleBuffer);
    device->Destroy(mvClusterBuildCmds);
    device->Destroy(mvSceneData);
    device->Destroy(tDepthTarget);

    device->Destroy(indirectSet);

    device->Destroy(cmdAlloc);
    device->Destroy(swapchain);
    device->Destroy();
}

void initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(width, height, "Scene Tree Demo", NULL, NULL);
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
    createResources();
    createRenderingObjects();
    mainLoop();
    cleanupLettuce();
    cleanupWindow();
    return 0;
}