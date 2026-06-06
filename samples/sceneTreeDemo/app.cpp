#include "Lettuce/Lettuce.hpp"
#include "glfw/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw/glfw3native.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <windows.h>

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

MemoryView mvSceneData;
MemoryViewInfo mviSceneData;

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

void createResources()
{
    mvSceneData = device->CreateMemoryView({ sizeof(SceneData), true });
    mviSceneData = device->GetMemoryViewInfo(mvSceneData);
}

void createRenderingObjects()
{
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

        // commands go here

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

    device->Destroy(mvSceneData);

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