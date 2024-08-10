//
// Created by piero on 10/02/2024.
//
#include <iostream>
#include <vector>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

import Lettuce;

void initWindow();
void endWindow();
void initLettuce();
void endLettuce();
void mainLoop();
void draw();

GLFWwindow *window;
const int width = 800;
const int height = 600;
Lettuce::Core::Instance instance;
Lettuce::Core::Device device;
Lettuce::Core::Swapchain swapchain;
Lettuce::Core::CommandPool pool;
Lettuce::Core::CommandList cmd;
Lettuce::Core::BSemaphore render;
Lettuce::Core::BSemaphore acquire;
std::vector<Lettuce::Core::TSemaphore> semaphores;
const int semaphoreCount = 1;

int main()
{
    initWindow();
    initLettuce();
    mainLoop();
    endLettuce();
    endWindow();
    return 0;
}

void initLettuce()
{
    instance._debug = true;
    instance.Create("ClearScreenSample", Lettuce::Core::Version{0, 1, 0, 0}, {});
    instance.CreateSurface(glfwGetWin32Window(window), GetModuleHandle(nullptr));
    auto gpus = instance.getGPUs();
    for (auto gpu : gpus)
    {
        std::cout << "available device: " << gpu.deviceName << std::endl;
        std::cout << "    graphics family: " << gpu.graphicsFamily.value() << std::endl;
        std::cout << "    present family : " << gpu.presentFamily.value() << std::endl;
        std::cout << "    gpu ptr:         " << gpu._pdevice << std::endl;
    }
    device.Create(instance, gpus.front(), {});
    swapchain.Create(device, width, height);
    render.Create(device);
    acquire.Create(device);
    pool.Build(device);
    semaphores = Lettuce::Core::TSemaphore::Create(device, semaphoreCount);
    cmd = pool.GetCommandLists()[0];
}

uint64_t value = 0;
void draw()
{
    auto graphicsFinished = value;
    auto allFinished = value+1; 
    swapchain.AcquireNextImage(acquire);
    cmd.Reset();
    cmd.Begin();
    cmd.BeginRendering(swapchain, 0.2, 0.2, 0.5);
    cmd.EndRendering(swapchain);
    cmd.End();
    std::vector<Lettuce::Core::CommandList> cmds {cmd};
    Lettuce::Core::CommandList::Send(device, cmds, Lettuce::Core::AccessStage::ColorAttachmentOutput, semaphores, {graphicsFinished}, {allFinished});
    value = allFinished;
    swapchain.Present(render);
    device.Wait();
}

void endLettuce()
{
    pool.Destroy();
    Lettuce::Core::TSemaphore::Destroy(semaphores);
    render.Destroy();
    acquire.Destroy();
    swapchain.Destroy();
    device.Destroy();
    instance.Destroy();
}

void mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        draw();
    }
}

void initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(width, height, "Lettuce ClearScreenSample", nullptr, nullptr);
}

void endWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}