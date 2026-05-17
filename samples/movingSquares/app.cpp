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
using namespace Lettuce::Composition;

GLFWwindow* window;

constexpr uint32_t width = 1366;
constexpr uint32_t height = 768;

std::unique_ptr<Device> device;
std::unique_ptr<Compositor> compositor;

Swapchain swapchain;
CommandAllocator cmdAlloc;

void mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        // device->NextFrame(swapchain);

        // device->Reset(cmdAlloc);
        // auto frame = device->GetCurrentRenderTarget(swapchain);
        // auto cmd = device->AllocateCommandBuffer(cmdAlloc);

        // std::array<std::span<CommandBuffer>, 1> cmds = { std::span(&cmd, 1) };

        // CommandBufferSubmitDesc submitDesc = {
        //     .queueType = QueueType::Graphics,
        //     .commandBuffers = std::span(cmds),
        //     .presentSwapchain = swapchain,
        // };

        // device->Submit(submitDesc);

        // device->DisplayFrame(swapchain);
        // device->WaitFor(QueueType::Graphics);
        std::println("app thread");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        glfwPollEvents();
    }
}

void initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(width, height, "Lettuce Composition Test", NULL, NULL);
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

    CommandAllocatorDesc cmdAllocDesc = {
        .queueType = QueueType::Graphics,
    };
    cmdAlloc = device->CreateCommandAllocator(cmdAllocDesc);
}

void cleanupLettuce()
{
    device->WaitFor(QueueType::Graphics);
    device->Destroy(cmdAlloc);
    device->Destroy(swapchain);
    device->Destroy();
}

void initCompositionEngine()
{
    CompositorDesc compositorCI = {
        .device = device.get(),
        .swapchain = swapchain,
        .maxAnimations = 1000,
        .maxLights = 64,
        .maxLinkedTextures = 64,
        .maxMaterials = 256,
        .maxVisuals = 1024,
    };
    compositor = std::make_unique<Compositor>();
    compositor->Create(compositorCI);

    auto geom = compositor->CreateGeometry(ImplicitGeometryDesc{ .shape = ImplicitShape::Rectangle });
    auto matColor = compositor->CreateMaterial(SolidColorMaterialDesc{ .color = {0.565, 0.933, 0.565} });

    auto myButtonVisual = compositor->CreateVisual(SpriteVisualDesc{ .size = {100, 50}, .geometry = geom, .material = matColor });

    auto hoverSpring = compositor->CreateAnimation(NaturalMotionAnimationDesc{ .stiffness = 300.0f, .damping = 15.0f });
    compositor->BindImplicitAnimation(myButtonVisual, AnimatableProperty::Scale, hoverSpring);

    // OnMouseEnter
    compositor->SetScale(myButtonVisual, { 1.1f, 1.1f });

    // auto missingGlyphs = RasterizeMissingGlyphs("🌍あ");
    // compositor.UploadToAtlasTexture(globalTextAtlas, missingGlyphs);
    // std::vector<AtlasInstance> glyphInstances;
    // glyphInstances.push_back({ uvH, rectH, {0,0,0,1}, AtlasInstanceFlags::None });
    // glyphInstances.push_back({ uvEarth, rectEarth, {1,1,1,1}, AtlasInstanceFlags::IsColorBitmap });

    // compositor.UpdateAtlasedInstances(myTextGeometry, glyphInstances);

    compositor->Commit();
}

void cleanupCompositionEngine()
{
    compositor->Destroy();
}

int main()
{
    setvbuf(stdout, nullptr, _IONBF, 0);
    initWindow();
    initLettuce();
    initCompositionEngine();
    mainLoop();
    cleanupCompositionEngine();
    cleanupLettuce();
    cleanupWindow();
    return 0;
}