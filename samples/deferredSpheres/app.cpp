#include "AppCommon.hpp"

#include <filesystem>
#include <memory>
#include <print>
#include <vector>

using namespace Lettuce::Core;
using namespace Lettuce::Rendering;

class App
{
private:
    GLFWwindow* window;

    uint32_t width = 1366;
    uint32_t height = 768;

    std::unique_ptr<Device> device;
    Swapchain swapchain;
    DescriptorTable descriptorTable;
    CommandAllocator cmdAlloc;

    std::unique_ptr<SceneView> scene;
    std::unique_ptr<Debug::DebugPass> debugPass;
    std::unique_ptr<Shapes::ShapeRenderPass> shapesPass;
    std::unique_ptr<DeferredPlus::MaterialShadingPass> materialShadingPass;
    GpuUniquePtr<SceneViewData> sceneViewData;
    GpuUniquePtr<uint32_t> pickInstanceData;
    GpuUniquePtr<SunLightData> sunLightData;
    GpuUniquePtr<SpotLightData> spotLightData;
    GpuUniquePtr<CascadeShadowMapData> shadowData;

    TextureView tDepthTarget;
    TextureView tPickTexture;

    Lettuce::Utils::FrameTimer timer;
    Lettuce::Utils::Camera3DDesc camera2Desc;
    Lettuce::Utils::Camera3D camera2; // explicit constructor

    double xprev = width / 2;
    double yprev = height / 2;
    uint32_t oldFbWidth = width;
    uint32_t oldFbHeight = height;
    bool wasMousePressed = false;
    void UpdateCamera2();
    void initLettuce();
    void createRenderingObjects();
    void loadModel();
    void mainLoop();
    void cleanupLettuce();
    void initWindow();
    void cleanupWindow();
public:
    explicit App() : camera2Desc{}, camera2(camera2Desc) {
        initWindow();
        initLettuce();
        createRenderingObjects();
        loadModel();
    }

    ~App() {
        cleanupLettuce();
        cleanupWindow();
    }

    void Run() {
        mainLoop();
    }
};

void App::UpdateCamera2()
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

    // Do not forget to update instance count :p
    *sceneViewData = camera2.Update({ wKeyPressed,aKeyPressed,sKeyPressed, dKeyPressed,static_cast<float>(dt) });
    sceneViewData->instanceCount = scene->GetInstanceTable().size();

    xprev = xpos;
    yprev = ypos;
    wasMousePressed = mousePressed;
}

void App::initLettuce()
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

void App::createRenderingObjects()
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
        .defaultClearValue = DepthStencilClear{ 1.0f, 0 },
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

    Shapes::ShapeRenderPassDesc shapePassDesc = {
        .device = *device,
        .descriptorTable = descriptorTable,
        .maxCommands = 10000,
    };
    shapesPass = std::make_unique<Shapes::ShapeRenderPass>(shapePassDesc);

    DeferredPlus::MaterialShadingPassDesc matPassDesc = {
        .device = *device,
        .descriptorTable = descriptorTable,
    };
    materialShadingPass = std::make_unique<DeferredPlus::MaterialShadingPass>(matPassDesc);
}

void App::loadModel()
{
    std::filesystem::path modelPath = "../../../../external/models/DragonAttenuation.glb";

    auto srcs = std::vector<GeometrySource>();
    srcs.push_back(Lettuce::Utils::AssetLoader::LoadGtlfAsGeometry(device.get(), modelPath.string()));

    SceneViewDesc sceneDesc = {
        .device = *device,
        .sources = srcs,
        .maxInstanceCount = 20,
    };
    scene = std::make_unique<SceneView>(sceneDesc);

    sceneViewData->instanceCount = scene->GetInstanceTable().size();
}

void App::mainLoop()
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
                .defaultClearValue = DepthStencilClear{ 1.0f, 0 },
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

void App::cleanupLettuce()
{
    device->WaitFor(QueueType::Graphics);

    scene.reset();
    debugPass.reset();
    materialShadingPass.reset();
    shapesPass.reset();

    sceneViewData.reset();
    pickInstanceData.reset();

    device->Destroy(descriptorTable);
    device->Destroy(tDepthTarget);
    device->Destroy(tPickTexture);

    device->Destroy(cmdAlloc);
    device->Destroy(swapchain);
    device.reset();
}

void App::initWindow()
{
    InitGlfw();
    window = glfwCreateWindow(width, height, "Deferred Spheres", NULL, NULL);
    glfwPollEvents();
}

void App::cleanupWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

int main()
{
    std::ios::sync_with_stdio(true);
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    auto app = App();
    app.Run();
    return 0;
}