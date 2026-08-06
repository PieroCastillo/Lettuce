#include "Lettuce/Lettuce.hpp"
#include "glfw/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw/glfw3native.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <windows.h>

#include <filesystem>
#include <memory>
#include <print>
#include <vector>

using namespace Lettuce::Core;
using namespace Lettuce::Quimera;
using namespace Lettuce::Rendering;

class App
{
    GLFWwindow* window;

    std::unique_ptr<Device> device;
    Swapchain swapchain;
    DescriptorTable descriptorTable;
    CommandAllocator cmdAlloc;

    std::unique_ptr<Surface> surface;
    Layout squareLayout;
    Layout circleLayout;
    Layout roundRectLayout;
    Geometry square;
    Geometry circle;
    Geometry roundRect;
    Brush redBrush;
    Brush blueBrush;
    Brush yellowBrush;

    std::unique_ptr<SceneView> scene;
    std::unique_ptr<Debug::DebugPass> debugPass;
    GpuUniquePtr<SceneViewData> sceneViewData;
    GpuUniquePtr<uint32_t> pickInstanceData;

    TextureView tDepthTarget;
    TextureView tPickTexture;

    Lettuce::Utils::FrameTimer timer;
    Lettuce::Utils::Camera3D camera2; // explicit constructor

    double xprev = width / 2;
    double yprev = height / 2;
    bool wasMousePressed = false;

    uint32_t width = 1366;
    uint32_t height = 768;
    uint32_t oldFbWidth = width;
    uint32_t oldFbHeight = height;

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

        sceneViewData->viewProj = camera2.Update({ wKeyPressed, aKeyPressed, sKeyPressed, dKeyPressed,static_cast<float>(dt) });

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
        device = std::make_unique<Device>(deviceCI);

        SwapchainDesc swapchainDesc = {
            .clipped = true,
            .windowPtr = &hwnd,
            .applicationPtr = &hmodule,
        };
        swapchain = device->CreateSwapchain(swapchainDesc);

        SurfaceDesc surfaceCI = {
            .device = *device,
            .maxImplicitGeometries = 10000,
            .maxBrushes = 10000,
            .maxDrawCommands = 10000,
        };
        surface = std::make_unique<Surface>(surfaceCI);

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

        TextureViewDesc pickDesc = {
            .width = width,
            .height = height,
            .depth = 1,
            .format = Format::Raw_R32_UInt,
            .mipCount = 1,
            .layerCount = 1,
            .cpuVisible = false,
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

        std::array<std::pair<uint32_t, TextureView>, 1> texDescs;
        texDescs[0] = { 0, tPickTexture };

        PushResourceDescriptorsDesc pushDtDesc = {
            .storageTextures = std::span(texDescs),
            .descriptorTable = descriptorTable,
        };
        device->PushResourceDescriptors(pushDtDesc);

        Debug::DebugPassDesc debugPassDesc = {
            .device = *device,
            .descriptorTable = descriptorTable,
            .maxCulledInstances = 10,
            .colorOutputFormat = device->GetRenderTargetFormat(swapchain),
        };
        debugPass = std::make_unique<Debug::DebugPass>(debugPassDesc);
    }

    void create2dResources()
    {
        LayoutDesc layoutDesc = {
            .position = float2(500, 420),
            .scale = float2(1),
        };
        squareLayout = surface->CreateLayout(layoutDesc);
        layoutDesc.position = { 760, 310 };
        circleLayout = surface->CreateLayout(layoutDesc);
        layoutDesc.position = { 830, 470 };
        roundRectLayout = surface->CreateLayout(layoutDesc);

        ImplicitGeometryDesc squareData = {
            .size = { 360, 360 },
        };
        square = surface->CreateGeometry(squareData);

        ImplicitGeometryDesc circleData = {
            { 320, 320 },
            160, 160, 160, 160,
        };
        circle = surface->CreateGeometry(circleData);

        ImplicitGeometryDesc yellowRoundRectData = {
            { 300, 180 },
            45, 45, 45, 45,
        };
        roundRect = surface->CreateGeometry(yellowRoundRectData);

        redBrush = surface->CreateBrush({ .color = Colors::Red });
        blueBrush = surface->CreateBrush({ .color = Colors::Blue });
        yellowBrush = surface->CreateBrush({ .color = Colors::Yellow });
    }

    void loadModel()
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

        Lettuce::Utils::Camera3DDesc camDesc;
        camera2 = Lettuce::Utils::Camera3D(camDesc);
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

            glfwGetFramebufferSize(window, (int*)&width, (int*)&height);

            if (width == 0 || height == 0)
            {
                glfwWaitEvents();
                continue;
            }

            auto fbSize = device->NextFrame(swapchain);

            if (fbSize.width != oldFbWidth || fbSize.height != oldFbHeight) [[unlikely]]
            {
                device->WaitFor(QueueType::Graphics);
                device->Destroy(tDepthTarget);
                device->Destroy(tPickTexture);

                TextureViewDesc pickDesc = {
                    .width = width,
                    .height = height,
                    .depth = 1,
                    .format = Format::Raw_R32_UInt,
                    .mipCount = 1,
                    .layerCount = 1,
                    .cpuVisible = false,
                };
                tPickTexture = device->CreateTextureView(pickDesc);

                RenderTargetDesc depthDesc = {
                    .width = width,
                    .height = height,
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

            // TextureToMemoryCopy tmPixelCopy =
            // {
            //     .srcTexture = tPickTexture,
            //     .dstMemory = mvPickInstanceData,
            //     .mipmapLevel = 0,
            //     .layerBaseLevel = 0,
            //     .layerCount = 1,
            //     .x = static_cast<uint32_t>(xCursorPos), .y = static_cast<uint32_t>(yCursorPos), .width = 1, .height = 1,
            // };

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

            BarrierDesc bColorOutputComp[] = { {
                .srcAccess = PipelineAccess::Write,
                .srcStage = PipelineStage::ColorAttachmentOutput,
                .dstAccess = PipelineAccess::Read,
                .dstStage = PipelineStage::Copy,
            }, };

            // cmd.ClearTexture(clearDesc);
            // cmd.Barrier(bCopyComp);

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
            };
            debugPass->Record(cmd, record);

            cmd.Barrier(bColorOutputComp);

            auto scmd = SurfaceCommandBuffer(*surface, cmd);
            scmd.Draw(1, square, blueBrush, squareLayout);
            scmd.Draw(2, circle, redBrush, circleLayout);
            scmd.Draw(3, roundRect, yellowBrush, roundRectLayout);
            scmd.DrawSurface({ frame, { 0, 0, width, height } });

            // if (isPressed)
            // {
            //     cmd.Barrier(bFragCopy);
            //     cmd.MemoryCopy(tmPixelCopy);
            // }
            // else
            // {
            //     *((uint32_t*)mviPickInstanceData.cpuAddress) = 0;
            // }

            std::array<std::span<CommandBuffer>, 1> cmds = { std::span(&cmd, 1) };

            CommandBufferSubmitDesc submitDesc = {
                .queueType = QueueType::Graphics,
                .commandBuffers = std::span(cmds),
                .presentSwapchain = swapchain,
            };

            device->Submit(submitDesc);

            device->DisplayFrame(swapchain);
            device->WaitFor(QueueType::Graphics);
            // if (isPressed)
            //     std::println("picked instance: {}", *((uint32_t*)mviPickInstanceData.cpuAddress) - 1);
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
        device->WaitFor(QueueType::Graphics);

        scene.reset();
        debugPass.reset();

        sceneViewData.reset();
        pickInstanceData.reset();

        device->Destroy(descriptorTable);
        device->Destroy(tDepthTarget);
        device->Destroy(tPickTexture);

        device->Destroy(cmdAlloc);
        surface.reset();
        device->Destroy(swapchain);
        device.reset();
    }

    void initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window = glfwCreateWindow(width, height, "Lettuce Editor", NULL, NULL);
    }

    void cleanupWindow()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

public:
    void Run()
    {
        initWindow();
        initLettuce();
        createRenderingObjects();
        create2dResources();
        loadModel();
        mainLoop();
        cleanupLettuce();
        cleanupWindow();
    }
};

int main()
{
    std::ios::sync_with_stdio(true);
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    auto app = App();
    app.Run();
    return 0;
}