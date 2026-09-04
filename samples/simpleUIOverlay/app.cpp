#include "Lettuce/Lettuce.hpp"
#include "glfw/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw/glfw3native.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <windows.h>

#include <algorithm>
#include <atomic>
#include <memory>
#include <print>
#include <string>
#include <vector>

using namespace Lettuce::Core;
using namespace Lettuce::Quimera;

GLFWwindow* window;

uint32_t width = 1366;
uint32_t height = 768;

std::unique_ptr<Device> device;
std::unique_ptr<Surface> surface;

Swapchain swapchain;
CommandAllocator cmdAlloc;

Layout squareLayout;
Layout circleLayout;
Layout roundRectLayout;
Geometry square;
Geometry circle;
Geometry roundRect;
Brush redBrush;
Brush blueBrush;
Brush yellowBrush;

void draw2dScene(CommandBuffer& lcmd, TextureView frame)
{
    auto cmd = SurfaceCommandBuffer(*surface, lcmd);
    cmd.Draw(1, square, blueBrush, squareLayout);
    cmd.Draw(2, circle, redBrush, circleLayout);
    cmd.Draw(3, roundRect, yellowBrush, roundRectLayout);
    cmd.DrawSurface({ frame, { 0, 0, width, height } });
}

void create2dResources()
{
    LayoutDesc layoutDesc = {
        .position = float2(500, 420),
        .scale = float2(1),
        .skew = float2(0),
        .anchorPoint = float2(0),
        .rotation = 0,
    };
    squareLayout = surface->CreateLayout(layoutDesc);
    layoutDesc.position = { 760, 310 };
    circleLayout = surface->CreateLayout(layoutDesc);
    layoutDesc.position = { 830, 470 };
    roundRectLayout = surface->CreateLayout(layoutDesc);

    ImplicitGeometryDesc squareData = {
        .size = { 360, 360 },
        .leftTopCornerRadious = 0,
        .leftBottomCornerRadious = 0,
        .rightTopCornerRadious = 0,
        .rightBottomCornerRadious = 0,
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

    SolidColorBrushDesc redData = {
        .color = { 1, 0, 0, 1 },
    };
    redBrush = surface->CreateBrush(redData);

    SolidColorBrushDesc blueData = {
        .color = { 0, 0, 1, 1 },
    };
    blueBrush = surface->CreateBrush(blueData);

    SolidColorBrushDesc yellowData = {
        .color = { 1, 1, 0, 1 },
    };
    yellowBrush = surface->CreateBrush(yellowData);
}

void cleanup2dResources()
{
}

void mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwGetFramebufferSize(window, (int*)&width, (int*)&height);

        if (width == 0 || height == 0)
        {
            glfwWaitEvents();
            continue;
        }

        auto fbSize = device->NextFrame(swapchain);

        device->Reset(cmdAlloc);
        auto frame = device->GetCurrentRenderTarget(swapchain);
        auto cmd = device->AllocateCommandBuffer(cmdAlloc);

        BarrierDesc bFragComp = {
            PipelineAccess::Write,
            PipelineStage::ColorAttachmentOutput,
            PipelineAccess::Read,
            PipelineStage::ComputeShader,
        };

        BarrierDesc bCompCopy = {
            PipelineAccess::Write,
            PipelineStage::ComputeShader,
            PipelineAccess::Read,
            PipelineStage::Copy,
        };

        AttachmentDesc colorAttachment[1] = {
            {
                .renderTarget = frame,
                .loadOp = LoadOp::Clear,
            }
        };

        RenderPassDesc renderPassDesc = {
            .width = fbSize.width,
            .height = fbSize.height,
            .colorAttachments = std::span(colorAttachment),
            .presentAttachmentIdx = 0,
        };

        cmd.BeginRendering(renderPassDesc);
        cmd.EndRendering();

        cmd.Barrier({ bFragComp });

        draw2dScene(cmd, frame);

        std::array<std::span<CommandBuffer>, 1> cmds = { std::span(&cmd, 1) };

        CommandBufferSubmitDesc submitDesc = {
            .queueType = QueueType::Graphics,
            .commandBuffers = std::span(cmds),
            .presentSwapchain = swapchain,
        };

        auto token = device->SubmitAsync(submitDesc);
        device->WaitFor(token);

        device->DisplayFrame(swapchain);

        glfwPollEvents();
    }
}

void initWindow()
{
    InitGlfw();
    window = glfwCreateWindow(width, height, "UI Overlay", NULL, NULL);
}
void cleanupWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void initLettuce()
{
    auto hwnd = glfwGetWin32Window(window);
    auto hmodule = GetModuleHandle(NULL);

    DeviceDesc deviceCI = {
        .preferDedicated = true,
    };
    device = std::make_unique<Device>(deviceCI);

    swapchain = device->CreateSwapchain(GetSwapchainDesc(window));

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

void cleanupLettuce()
{
    device->WaitFor(QueueType::Graphics);
    surface.reset();
    device->Destroy(cmdAlloc);
    device->Destroy(swapchain);
    device.reset();
}

int main()
{
    setvbuf(stdout, nullptr, _IONBF, 0);
    initWindow();
    initLettuce();
    create2dResources();
    mainLoop();
    cleanup2dResources();
    cleanupLettuce();
    cleanupWindow();
    return 0;
}