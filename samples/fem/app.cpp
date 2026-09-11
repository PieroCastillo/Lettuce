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

// structs
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

// fields
GLFWwindow* window;

uint32_t width = 1366;
uint32_t height = 768;

std::unique_ptr<Device> device;
Swapchain swapchain;
DescriptorTable descriptorTable;
CommandAllocator cmdAlloc;
CommandAllocator copyCmdAlloc;

GpuUniquePtr<SceneViewData> sceneViewData;

TextureView tDepthTarget;

Lettuce::Utils::FrameTimer timer;
Lettuce::Utils::Camera3DDesc camera2Desc;
Lettuce::Utils::Camera3D camera2(camera2Desc); // explicit constructor

Pipeline pFiniteElements;
Pipeline pRender;

GpuUniquePtr<SimulationInfo> simulationInfo;
GpuUploadVector<Node> nodes;
GpuUploadVector<Element> elements;

GpuUploadVector<float3> positions;
GpuUploadVector<uint32_t> indices;

double xprev = width / 2;
double yprev = height / 2;
bool wasMousePressed = false;

static Element makeElement(uint32_t i0, uint32_t i1, uint32_t i2, uint32_t i3, const std::vector<Node>& nodes)
{
    Element e{};

    e.nodeIdxs[0] = i0;
    e.nodeIdxs[1] = i1;
    e.nodeIdxs[2] = i2;
    e.nodeIdxs[3] = i3;

    const auto& x0 = nodes[i0].position;
    const auto& x1 = nodes[i1].position;
    const auto& x2 = nodes[i2].position;
    const auto& x3 = nodes[i3].position;

    const auto a = x1 - x0;
    const auto b = x2 - x0;
    const auto c = x3 - x0;

    const float detJ = dot(a, cross(b, c));

    e.volume = std::abs(detJ) / 6.0f;

    // Gradients
    // N_i = a_i + b_i*x + c_i*y + d_i*z
    //
    // ∇Ni = (dNi/dx, dNi/dy, dNi/dz)

    const float invDet = 1.0f / detJ;

    float3 gradN[4];

    gradN[0] = cross(x2 - x1, x3 - x1) * invDet;
    gradN[1] = cross(x3 - x0, x2 - x0) * invDet;
    gradN[2] = cross(x1 - x0, x3 - x0) * invDet;
    gradN[3] = cross(x2 - x0, x1 - x0) * invDet;

    // B = 6x12
    //
    // [ dN/dx   0      0   ]
    // [ 0       dN/dy  0   ]
    // [ 0       0      dN/dz]
    // [ dN/dy   dN/dx  0   ]
    // [ 0       dN/dz  dN/dy]
    // [ dN/dz   0      dN/dx]

    for (int i = 0; i < 4; ++i)
    {
        const int j = i * 3;

        const float dx = gradN[i].x;
        const float dy = gradN[i].y;
        const float dz = gradN[i].z;

        e.B[0][j + 0] = dx;
        e.B[0][j + 1] = 0.0f;
        e.B[0][j + 2] = 0.0f;

        e.B[1][j + 0] = 0.0f;
        e.B[1][j + 1] = dy;
        e.B[1][j + 2] = 0.0f;

        e.B[2][j + 0] = 0.0f;
        e.B[2][j + 1] = 0.0f;
        e.B[2][j + 2] = dz;

        e.B[3][j + 0] = dy;
        e.B[3][j + 1] = dx;
        e.B[3][j + 2] = 0.0f;

        e.B[4][j + 0] = 0.0f;
        e.B[4][j + 1] = dz;
        e.B[4][j + 2] = dy;

        e.B[5][j + 0] = dz;
        e.B[5][j + 1] = 0.0f;
        e.B[5][j + 2] = dx;
    }

    return e;
}

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

    cmdAllocDesc.queueType = QueueType::Copy;
    copyCmdAlloc = device->CreateCommandAllocator(cmdAllocDesc);
}

void createRenderingObjects()
{
    // load buffers
    sceneViewData = GpuUniquePtr<SceneViewData>(*device);

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

    std::array<Format, 1> formatArr = { device->GetRenderTargetFormat(swapchain) };

    auto shadersPath = "samples/fem/fem.spv";
    auto shaders = Lettuce::Utils::AssetLoader::LoadSpirv(device.get(), shadersPath);

    ComputePipelineDesc pFemDesc = {
        .compEntryPoint = "compMain",
        .compShaderBinary = shaders,
        .descriptorTable = descriptorTable,
    };
    pFiniteElements = device->CreatePipeline(pFemDesc);

    PrimitiveShadingPipelineDesc pRenderDesc = {
        .vertEntryPoint = "vertMain",
        .fragEntryPoint = "fragMain",
        .vertShaderBinary = shaders,
        .fragShaderBinary = shaders,
        .colorAttachmentFormats = std::span(formatArr),
        .descriptorTable = descriptorTable,
    };
    pRender = device->CreatePipeline(pRenderDesc);

    device->Destroy(shaders);
}

void loadModel()
{
    simulationInfo = GpuUniquePtr<SimulationInfo>(*device);

    std::filesystem::path modelPath = "../../../../external/models/connectingRod.stl";
    std::vector<float3> tempPositions;
    std::vector<uint32_t> tempIndices;

    try {
        stl_reader::StlMesh<float, uint32_t> mesh(modelPath.string());
        tempPositions.resize(mesh.num_vrts());
        memcpy((void*)tempPositions.data(), (void*)mesh.raw_coords(), mesh.num_vrts() * sizeof(float3));

        tempIndices.resize(mesh.num_tris() * 3);
        memcpy((void*)tempIndices.data(), (void*)mesh.raw_tris(), mesh.num_tris() * 3 * sizeof(uint32_t));
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

    V.resize(tempPositions.size(), 3);
    for (size_t i = 0; i < tempPositions.size(); ++i)
    {
        V(i, 0) = tempPositions[i].x;
        V(i, 1) = tempPositions[i].y;
        V(i, 2) = tempPositions[i].z;
    }

    F.resize(tempIndices.size() / 3, 3);
    for (size_t i = 0; i < tempIndices.size(); i += 3)
    {
        F(i / 3, 0) = tempIndices[i];
        F(i / 3, 1) = tempIndices[i + 1];
        F(i / 3, 2) = tempIndices[i + 2];
    }

    // Tetrahedralize the interior
    igl::copyleft::tetgen::tetrahedralize(V, F, "pq1.414Y", TV, TT, TF);
    // Compute barycenters
    igl::barycenter(TV, TT, B);

    std::vector<Node> tempNodes(TV.rows());
    std::vector<Element> tempElements(TT.rows());

    for (Eigen::Index i = 0; i < TV.rows(); ++i)
    {
        tempNodes[i].position = {
            static_cast<float>(TV(i, 0)),
            static_cast<float>(TV(i, 1)),
            static_cast<float>(TV(i, 2))
        };

        tempNodes[i].displacement = { 0.0f, 0.0f, 0.0f };
        tempNodes[i].force = { 0.0f, 0.0f, 0.0f };
    }

    for (Eigen::Index i = 0; i < TT.rows(); ++i)
    {
        tempElements[i] = makeElement(
            static_cast<uint32_t>(TT(i, 0)),
            static_cast<uint32_t>(TT(i, 1)),
            static_cast<uint32_t>(TT(i, 2)),
            static_cast<uint32_t>(TT(i, 3)),
            tempNodes
        );
    }

    elements = GpuUploadVector<Element>(*device, tempElements.size());
    nodes = GpuUploadVector<Node>(*device, tempNodes.size());
    positions = GpuUploadVector<float3>(*device, tempPositions.size());
    indices = GpuUploadVector<uint32_t>(*device, tempIndices.size());

    elements.append(tempElements);
    nodes.append(tempNodes);
    positions.append(tempPositions);
    indices.append(tempIndices);

    elements.Upload(copyCmdAlloc);
    nodes.Upload(copyCmdAlloc);
    positions.Upload(copyCmdAlloc);
    indices.Upload(copyCmdAlloc);

    // steel ISO 42CrMo4
    simulationInfo->elementCount = elements.size();
    simulationInfo->nodeCount = nodes.size();
    simulationInfo->iterationCount = 15;
    simulationInfo->material.density = 7850; // kg/m3
    simulationInfo->material.young = 2.1e+11; // N/m2
    simulationInfo->material.poisson = 0.3; // dimensionless

    /* todo:
    - setup nodes and elements
    - initialize nodes and elements
    */

    // // load model into gpu memory
}

void execFem()
{
    auto cmd = device->AllocateCommandBuffer(cmdAlloc);

    auto allocs = std::vector<PushAllocationBinding>{
          sceneViewData.getView(),
          simulationInfo.getView(),
          nodes.getView(),
          elements.getView(),
    };
    PushAllocationsDesc pushDesc = {
        .allocations = allocs,
        .descriptorTable = descriptorTable,
    };

    cmd.BindDescriptorTable(descriptorTable, PipelineBindPoint::Compute);
    cmd.BindPipeline(pFiniteElements);
    cmd.PushAllocations(pushDesc);
    cmd.Dispatch(elements.size() / 32, 1, 1);

    std::array<std::span<CommandBuffer>, 1> cmds = { std::span(&cmd, 1) };

    CommandBufferSubmitDesc submitDesc = {
        .queueType = QueueType::Graphics,
        .commandBuffers = std::span(cmds),
        .presentSwapchain = swapchain,
    };

    device->Submit(submitDesc);
    device->WaitFor(QueueType::Graphics);
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

        std::vector<AttachmentDesc> colorAttachments = { {
            .renderTarget = frame,
            .loadOp = LoadOp::Clear,
        } };

        RenderPassDesc renderPassDesc = {
            .width = fbSize.width,
            .height = fbSize.height,
            .colorAttachments = std::span(colorAttachments),
            .presentAttachmentIdx = 0,
        };

        auto allocs = std::vector<PushAllocationBinding>{
            sceneViewData.getView(),
            simulationInfo.getView(),
            nodes.getView(),
            elements.getView(),
            positions.getView(),
            indices.getView(),
        };
        PushAllocationsDesc pushDesc = {
            .allocations = allocs,
            .descriptorTable = descriptorTable,
        };

        cmd.BeginRendering(renderPassDesc);
        cmd.BindDescriptorTable(descriptorTable, PipelineBindPoint::Graphics);
        cmd.BindPipeline(pRender);
        cmd.PushAllocations(pushDesc);
        cmd.Draw(indices.size(), 1);
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

    simulationInfo.reset();

    device->Destroy(pRender);
    device->Destroy(pFiniteElements);
    device->Destroy(descriptorTable);
    device->Destroy(tDepthTarget);
    sceneViewData.reset();

    device->Destroy(copyCmdAlloc);
    device->Destroy(cmdAlloc);
    device->Destroy(swapchain);
    elements.reset();
    nodes.reset();
    positions.reset();
    indices.reset();
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
    loadModel();
    createRenderingObjects();
    execFem();
    mainLoop();
    cleanupLettuce();
    cleanupWindow();
    return 0;
}