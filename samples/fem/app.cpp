#include "AppCommon.hpp"

#include <filesystem>
#include <memory>
#include <print>
#include <vector>

#include <igl/copyleft/tetgen/tetrahedralize.h>
#include <igl/readOFF.h>
#include <igl/barycenter.h>
#include <stl_reader.hpp>

using namespace Lettuce::Core;
using namespace Lettuce::Rendering;

GLFWwindow* window;

uint32_t width = 1366;
uint32_t height = 768;

std::unique_ptr<Device> device;
Swapchain swapchain;
DescriptorTable descriptorTable;
CommandAllocator cmdAlloc;

std::unique_ptr<SceneView> scene;
std::unique_ptr<Debug::DebugPass> debugPass;
GpuUniquePtr<SceneViewData> sceneViewData;
GpuUniquePtr<uint32_t> pickInstanceData;

TextureView tDepthTarget;
TextureView tPickTexture;

Lettuce::Utils::FrameTimer timer;
Lettuce::Utils::Camera3DDesc camera2Desc;
Lettuce::Utils::Camera3D camera2(camera2Desc); // explicit constructor

struct Material
{
    float young;   // E
    float poisson; // nu
    float density; // uniform
};

struct SimulationInfo
{
    uint32_t elementCount;
    uint32_t nodeCount;
    uint32_t iterationCount;
    Material material;
};

struct Node
{
    float3 position;
    float3 displacement;
    float3 force;
    uint32_t constraints; // bits: [highest, ... | X | Y | Z , lowest]
};

struct Element
{
    uint32_t nodeIdxs[4];

    float volume;
    float B[6][12];
};

GpuUploadVector<Node> nodes;
GpuUploadVector<Element> elemments;

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

    sceneViewData->viewProj = camera2.Update({ wKeyPressed,aKeyPressed,sKeyPressed, dKeyPressed,static_cast<float>(dt) });

    xprev = xpos;
    yprev = ypos;
    wasMousePressed = mousePressed;
}

void initLettuce()
{
    DeviceDesc deviceCI = {
        .preferDedicated = true,
    };
    device = std::make_unique<Device>(deviceCI);

    glfwGetFramebufferSize(window, (int*)&width, (int*)&height);
    std::println("glfw fb size: {},{}", width, height);
    swapchain = device->CreateSwapchain(GetSwapchainDesc(window));

    CommandAllocatorDesc cmdAllocDesc = {
        .queueType = QueueType::Graphics,
    };
    cmdAlloc = device->CreateCommandAllocator(cmdAllocDesc);
}

void createRenderingObjects()
{
    // load buffers
    sceneViewData = GpuUniquePtr<SceneViewData>(*device);
    pickInstanceData = GpuUniquePtr<uint32_t>(*device);

    RenderTargetDesc pickDesc = {
        .width = width,
        .height = height,
        .type = RenderTargetType::ColorRGB_R32UInt,
        .defaultClearValue = ColorClear{},
    };
    tPickTexture = device->CreateTextureView(pickDesc);

    RenderTargetDesc depthDesc = {
        .width = width,
        .height = height,
        .type = RenderTargetType::Depth_D32,
        .defaultClearValue = DepthStencilClear {1.0f, 0},
    };
    tDepthTarget = device->CreateTextureView(depthDesc);

    // load pipelines
    DescriptorTableDesc descriptorTableDesc = { 4,4,4 };
    descriptorTable = device->CreateDescriptorTable(descriptorTableDesc);

    Debug::DebugPassDesc debugPassDesc = {
        .device = *device,
        .descriptorTable = descriptorTable,
        .maxCulledInstances = 10,
        .colorOutputFormat = device->GetRenderTargetFormat(swapchain),
    };
    debugPass = std::make_unique<Debug::DebugPass>(debugPassDesc);
}

void loadModel()
{
    std::filesystem::path modelPath = "../../../../external/models/connectingRod.stl";
    std::vector<float3> positions;
    std::vector<uint32_t> indices;

    try {
        stl_reader::StlMesh<float, uint32_t> mesh(modelPath.string());
        positions.resize(mesh.num_vrts());
        memcpy((void*)positions.data(), (void*)mesh.raw_coords(), mesh.num_vrts() * sizeof(float3));

        indices.resize(mesh.num_tris() * 3);
        memcpy((void*)indices.data(), (void*)mesh.raw_tris(), mesh.num_tris() * 3 * sizeof(uint32_t));
    }
    catch (std::exception& e) {
        std::cout << "bad copy | " << e.what() << std::endl;
        std::abort();
    }

    // create tetrahedralized representation
    Eigen::MatrixXd V;
    Eigen::MatrixXi F;
    Eigen::MatrixXd B;

    Eigen::MatrixXd TV;
    Eigen::MatrixXi TT;
    Eigen::MatrixXi TF;

    V.resize(positions.size(), 3);
    for (size_t i = 0; i < positions.size(); ++i)
    {
        V(i, 0) = positions[i].x;
        V(i, 1) = positions[i].y;
        V(i, 2) = positions[i].z;
    }

    F.resize(indices.size() / 3, 3);
    for (size_t i = 0; i < indices.size(); i += 3)
    {
        F(i / 3, 0) = indices[i];
        F(i / 3, 1) = indices[i + 1];
        F(i / 3, 2) = indices[i + 2];
    }

    // Tetrahedralize the interior
    igl::copyleft::tetgen::tetrahedralize(V, F, "pq1.414Y", TV, TT, TF);
    // Compute barycenters
    igl::barycenter(TV, TT, B);

    std::println("expected abort");
    std::abort();

    /* todo:
    - setup pipelines
    - setup nodes and elements
    - initialize nodes and elements
    */

    // // load model into gpu memory
    // auto srcs = std::vector<GeometrySource>();
    // srcs.push_back(std::move(geometrySrc));

    // SceneViewDesc sceneDesc = {
    //     .device = *device,
    //     .sources = srcs,
    //     .maxInstanceCount = 20,
    // };
    // scene = std::make_unique<SceneView>(sceneDesc);

    // sceneViewData->instanceCount = scene->GetInstanceTable().size();
}

uint32_t oldFbWidth = width;
uint32_t oldFbHeight = height;
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

        glfwGetFramebufferSize(window, (int*)&width, (int*)&height);

        if (width == 0 || height == 0)
        {
            glfwWaitEvents();
            continue;
        }

        auto fbSize = device->NextFrame(swapchain, width, height);

        if (fbSize.width != oldFbWidth || fbSize.height != oldFbHeight) [[unlikely]]
        {
            device->WaitFor(QueueType::Graphics);
            device->Destroy(tDepthTarget);
            device->Destroy(tPickTexture);

            RenderTargetDesc pickDesc = {
                .width = fbSize.width,
                .height = fbSize.height,
                .type = RenderTargetType::ColorRGB_R32UInt,
                .defaultClearValue = ColorClear{},
            };
            tPickTexture = device->CreateTextureView(pickDesc);

            RenderTargetDesc depthDesc = {
                .width = fbSize.width,
                .height = fbSize.height,
                .type = RenderTargetType::Depth_D32,
                .defaultClearValue = DepthStencilClear {1.0f, 0},
            };
            tDepthTarget = device->CreateTextureView(depthDesc);

            oldFbWidth = fbSize.width;
            oldFbHeight = fbSize.height;
        }

        device->Reset(cmdAlloc);
        auto frame = device->GetCurrentRenderTarget(swapchain);
        auto cmd = device->AllocateCommandBuffer(cmdAlloc);

        Debug::DebugPassRecordDesc record = {
            .fbWidth = fbSize.width,
            .fbHeight = fbSize.height,
            .sceneViewData = GpuSpan(sceneViewData),
            .positions = scene->GetPositionsView(),
            .indices = scene->GetIndicesView(),
            .clusters = scene->GetClustersView(),
            .meshes = scene->GetMeshesView(),
            .culledInstances = scene->GetInstanceTable(),
            .rtColorOutput = frame,
            .rtDepth = tDepthTarget,
            .rtPick = tPickTexture,
        };
        debugPass->Record(cmd, record);

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

    scene.reset();
    debugPass.reset();

    sceneViewData.reset();
    pickInstanceData.reset();

    device->Destroy(descriptorTable);
    device->Destroy(tDepthTarget);
    device->Destroy(tPickTexture);

    device->Destroy(cmdAlloc);
    device->Destroy(swapchain);
    device.reset();
}

void initWindow()
{
    InitGlfw();
    window = glfwCreateWindow(width, height, "Finite Elements Method Sample", NULL, NULL);
    glfwPollEvents();
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
    mainLoop();
    cleanupLettuce();
    cleanupWindow();
    return 0;
}