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

constexpr uint32_t width = 1366;
constexpr uint32_t height = 768;

std::unique_ptr<Device> device;
std::unique_ptr<Surface> surface;

Swapchain swapchain;
CommandAllocator cmdAlloc;

Geometry square;
Geometry circle;
Geometry roundRect;
Brush redBrush;
Brush blueBrush;
Brush yellowBrush;

inline glm::mat3 createTransform2D(float angleRad, float x, float y,
    float scaleX = 1.0f, float scaleY = 1.0f,
    float skewX = 0.0f, float skewY = 0.0f)
{
    auto c = std::cos(angleRad);
    auto s = std::sin(angleRad);

    return glm::mat3(
        /* column 0 */ scaleX * (c + s * skewY), scaleX * (s - c * skewY), 0.0f,
        /* column 1 */ scaleY * (c * skewX - s), scaleY * (s * skewX + c), 0.0f,
        /* column 2 */ x, y, 1.0f,
    );
}


void draw2dScene(CommandBuffer& lcmd, TextureView frame)
{
    auto cmd = SurfaceCommandBuffer(*surface, lcmd);
    cmd.Draw(1, square, blueBrush, createTransform2D(0, 500, 420));
    cmd.Draw(2, circle, redBrush, createTransform2D(0, 760, 310));
    cmd.Draw(3, roundRect, yellowBrush, createTransform2D(0, 830, 470));
    cmd.DrawSurface({ frame, { 0, 0, width, height } });
}

void create2dResources()
{
    ImplicitGeometryDesc squareData = {
          { 360, 360 },
          0, 0, 0, 0,
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
        { 1, 0, 0, 1 },
    };
    redBrush = surface->CreateBrush(redData);

    SolidColorBrushDesc blueData = {
        { 0, 0, 1, 1 },
    };
    blueBrush = surface->CreateBrush(blueData);

    SolidColorBrushDesc yellowData = {
        { 1, 1, 0, 1 },
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
        device->NextFrame(swapchain);

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
            .width = width,
            .height = height,
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

        device->Submit(submitDesc);

        device->DisplayFrame(swapchain);
        device->WaitFor(QueueType::Graphics);

        glfwPollEvents();
    }
}

void initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(width, height, "Lettuce Quimera 2D Test", NULL, NULL);
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
    device = std::make_unique<Device>();
    device->Create(deviceCI);

    SwapchainDesc swapchainDesc = {
        .width = width,
        .height = height,
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
    surface = std::make_unique<Surface>();
    surface->Create(surfaceCI);

    CommandAllocatorDesc cmdAllocDesc = {
        .queueType = QueueType::Graphics,
    };
    cmdAlloc = device->CreateCommandAllocator(cmdAllocDesc);
}

void cleanupLettuce()
{
    device->WaitFor(QueueType::Graphics);
    surface->Destroy();
    device->Destroy(cmdAlloc);
    device->Destroy(swapchain);
    device->Destroy();
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