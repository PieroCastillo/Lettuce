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

    Sampler sampler;

    GpuUniquePtr<SceneViewData> sceneViewData;
    GpuUniquePtr<uint32_t> pickInstanceData;
    GpuUniquePtr<MaterialShadingData> materialShadingData;
    GpuUniquePtr<ProceduralInstanceData> instanceData;

    GpuUniquePtr<SunLightData> sunLightData;
    GpuUniquePtr<SpotLightData> spotLightData;
    GpuUniquePtr<CascadeShadowMapData> shadowData;

    GpuMappedVector<MaterialPBRStorage> materials;
    GpuMappedVector<ProceduralInstanceStorage> instances;
    GpuMappedVector<SphereStorage> spheres;
    GpuMappedVector<PrismStorage> prisms;
    GpuMappedVector<CylinderStorage> cylinders;

    TextureView tDepthTarget;
    TextureView tGeometryTarget;
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
    void loadGpuPtrs();
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
        loadGpuPtrs();
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
    // load targets
    RenderTargetDesc pickDesc = {
        .width = width,
        .height = height,
        .type = RenderTargetType::ColorRGB_R32UInt,
        .defaultClearValue = ColorClear{},
    };
    tPickTexture = device->CreateTextureView(pickDesc);

    RenderTargetDesc geometryDesc = {
        .width = width,
        .height = height,
        .type = RenderTargetType::ColorRGBA_sRGB,
        .defaultClearValue = ColorClear{},
    };
    tGeometryTarget = device->CreateTextureView(geometryDesc);

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

    // load samplers
    sampler = device->CreateSampler({
        Filter::Linear, Filter::Linear, Filter::Linear,
        SamplerAddressMode::Repeat,  SamplerAddressMode::Repeat,  SamplerAddressMode::Repeat,
        1.0f, false });
}

struct InstanceDesc
{
    ProceduralShapeGeometryType type;
    uint32_t geometry;
    uint32_t material;
    glm::vec3 position;
    glm::vec3 scale;
};

void App::loadGpuPtrs()
{
    const auto shapesCollection = std::array
    {
        InstanceDesc{ Sphere,   0, 1,{ -1.8f, 0.75f, 0.0f },{ 1.0f, 1.0f, 1.0f } },
        InstanceDesc{ Prism,    0, 2,{ 0.0f, 0.65f, 0.0f },{ 1.0f, 1.0f, 1.0f } },
        InstanceDesc{ Cylinder, 0, 3,{ 1.8f, 0.75f, 0.0f },{ 1.0f, 1.0f, 1.0f } }
    };

    sceneViewData = GpuUniquePtr<SceneViewData>(*device);
    pickInstanceData = GpuUniquePtr<uint32_t>(*device);
    materialShadingData = GpuUniquePtr<MaterialShadingData>(*device);
    instanceData = GpuUniquePtr<ProceduralInstanceData>(*device);

    sunLightData = GpuUniquePtr<SunLightData>(*device);
    spotLightData = GpuUniquePtr<SpotLightData>(*device);
    shadowData = GpuUniquePtr<CascadeShadowMapData>(*device);

    materials = GpuMappedVector<MaterialPBRStorage>(*device, 4);
    instances = GpuMappedVector<ProceduralInstanceStorage>(*device, shapesCollection.size());
    spheres = GpuMappedVector<SphereStorage>(*device, 1);
    prisms = GpuMappedVector<PrismStorage>(*device, 1);
    cylinders = GpuMappedVector<CylinderStorage>(*device, 1);

    uint32_t depthTexHeapIdx = 0;
    uint32_t gbufferTexHeapIdx = 1;

    materialShadingData->depthTextureIdx = depthTexHeapIdx;
    materialShadingData->gbufferTextureIdx = gbufferTexHeapIdx;

    std::array<std::pair<uint32_t, TextureView>, 2> texDescs;
    texDescs[0] = { depthTexHeapIdx, tDepthTarget };
    texDescs[1] = { gbufferTexHeapIdx, tGeometryTarget };

    std::array<std::pair<uint32_t, Sampler>, 1> samplers;
    // samplers[0] = { 0, sampler };

    // push texture descriptors
    PushResourceDescriptorsDesc pushDesc = {
        .sampledTextures = std::span(texDescs),
        .samplers = std::span(samplers),
        .descriptorTable = descriptorTable,
    };
    device->PushResourceDescriptors(pushDesc);

    materials[0] = {
        .baseColor = { 0.72f, 0.72f, 0.72f, 1.0f },
        .emissive = { 0.0f, 0.0f, 0.0f },
        .metallic = 0.0f,
        .roughness = 0.85f
    };

    materials[1] = {
        .baseColor = { 0.85f, 0.08f, 0.04f, 1.0f },
        .emissive = { 0.0f, 0.0f, 0.0f },
        .metallic = 0.1f,
        .roughness = 0.3f
    };

    materials[2] = {
        .baseColor = { 0.04f, 0.25f, 0.85f, 1.0f },
        .emissive = { 0.0f, 0.0f, 0.0f },
        .metallic = 0.85f,
        .roughness = 0.2f
    };

    materials[3] = {
        .baseColor = { 0.9f, 0.65f, 0.08f, 1.0f },
        .emissive = { 0.0f, 0.0f, 0.0f },
        .metallic = 0.65f,
        .roughness = 0.35f
    };

    // Procedural geometries
    spheres[0] = {
        .radius = 0.75f,
        .longitudeSegments = 48,
        .latitudeSegments = 24,
        ._padding = 0
    };

    prisms[0] = {
        .radius = 0.7f,
        .height = 1.3f,
        .sides = 6,
        ._padding = 0
    };

    cylinders[0] = {
        .radiusTop = 0.4f,
        .radiusBottom = 0.4f,
        .height = 1.5f,
        .radialSegments = 48
    };

    for (uint32_t i = 0; i < shapesCollection.size(); ++i)
    {
        const auto& desc = shapesCollection[i];
        auto transform = glm::translate(glm::mat4(1.0f), desc.position) * glm::scale(glm::mat4(1.0f), desc.scale);

        instances[i] = {
            .transform = transform,
            .invTransform = glm::inverse(transform),
            .geometryType = static_cast<uint32_t>(desc.type),
            .geometryIdx = desc.geometry,
            .materialIdx = desc.material,
            .flags = 0,
        };
    }

    // instance metadata
    *instanceData = {
        .count = shapesCollection.size(),
        .sphereCount = 1,
        .prismCount = 1,
        .cylinderCount = 1
    };

    // lights
    *sunLightData = {
        .direction = glm::normalize(glm::vec3(-0.5f, 0.8f, 0.4f)),
        .intensity = 3.0f,
        .color = { 1.0f, 0.95f, 0.85f },
        ._padding = 0.0f
    };

    *spotLightData = {};
    spotLightData->count = 2;

    spotLightData->lights[0] = {
        .position = { -3.0f, 3.5f, 3.0f },
        .range = 10.0f,
        .direction = glm::normalize(glm::vec3(0.7f, -0.7f, -0.8f)),
        .intensity = 20.0f,
        .color = { 0.35f, 0.55f, 1.0f },
        .innerCos = glm::cos(glm::radians(20.0f)),
        .outerCos = glm::cos(glm::radians(35.0f)),
        .padding = {}
    };

    spotLightData->lights[1] = {
        .position = { 3.0f, 3.5f, 3.0f },
        .range = 10.0f,
        .direction = glm::normalize(glm::vec3(-0.7f, -0.7f, -0.8f)),
        .intensity = 20.0f,
        .color = { 1.0f, 0.55f, 0.25f },
        .innerCos = glm::cos(glm::radians(20.0f)),
        .outerCos = glm::cos(glm::radians(35.0f)),
        .padding = {}
    };

    *shadowData = {};
    shadowData->cascadeCount = 0;
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

            RenderTargetDesc geometryDesc = {
                .width = fbSize.width,
                .height = fbSize.height,
                .type = RenderTargetType::ColorRGBA_sRGB,
                .defaultClearValue = ColorClear{},
            };
            tGeometryTarget = device->CreateTextureView(geometryDesc);

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

        Shapes::ShapeRenderPassRecordDesc shapeRec = {
            .fbWidth = fbSize.width,
            .fbHeight = fbSize.height,
            .sceneViewData = GpuSpan(sceneViewData),

            .rtColorOutput = tGeometryTarget,
            .rtDepth = tDepthTarget,
            .rtPick = tPickTexture,
            .rtLoadOp = LoadOp::Clear,
        };
        // shapesPass->Record(cmd, shapeRec);

        DeferredPlus::MaterialShadingPassRecordDesc materialRec = {
            .fbWidth = fbSize.width,
            .fbHeight = fbSize.height,
            .sceneViewData = GpuSpan(sceneViewData),

            .rtColorOutput = frame,
            .rtDepth = tDepthTarget,
            .rtPick = tPickTexture,
        };
        // materialShadingPass->Record(cmd, materialRec);

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

    // TODO: add dtors for datas, storages and sampler
    
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