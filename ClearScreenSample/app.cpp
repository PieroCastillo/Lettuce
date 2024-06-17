//
// Created by piero on 10/02/2024.
//
#include <iostream>
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

GLFWwindow* window;
const int width = 800;
const int height = 600;
Lettuce::Core::Instance instance;
Lettuce::Core::Device device;
Lettuce::Core::Swapchain swapchain;
Lettuce::Core::CommandList commandList;
Lettuce::Core::SynchronizationStructure sync;
const int acquireImageSemaphoreIndex = 0;
const int renderSemaphoreIndex = 1;
const int fenceIndex = 0;

int main()
{
    initWindow();
    initLettuce();
    mainLoop();
    endLettuce();
    endWindow();
    return 0;
}

void initLettuce() {
    instance._debug = true;
    instance.Create("ClearScreenSample", Lettuce::Core::Version{ 0,1,0,0 }, {});
    instance.CreateSurface(glfwGetWin32Window(window), GetModuleHandle(nullptr));
    auto gpus = instance.getGPUs();
    for(auto gpu : gpus) {
        std::cout << "available device: " << gpu.deviceName << std::endl;
        std::cout << "    graphics family: " << gpu.graphicsFamily.value() << std::endl;
        std::cout << "    present family : " << gpu.presentFamily.value() << std::endl;
        std::cout << "    gpu ptr:         " << gpu._pdevice << std::endl;
    }
    device.Create(instance, gpus.front(), {});
    sync.Create(device, 1, 2);  
    swapchain.Create(device, sync, width, height);
    commandList.Create(device, sync);
}
   
void draw(){
    std::cout << "frame" << std::endl;
    sync.WaitForFence(fenceIndex);
    sync.ResetAllFences();
    swapchain.AcquireNextImage(acquireImageSemaphoreIndex);
    commandList.Reset();
    commandList.Begin();
    //TODO: fix these
    // commandList.BeginRendering(swapchain, 0.2,0.5,0.3);
    // commandList.EndRendering();
    commandList.End();
    commandList.Send(acquireImageSemaphoreIndex, renderSemaphoreIndex, fenceIndex);
    std::cout << "swp present" << std::endl;
    swapchain.Present(renderSemaphoreIndex);
    device.Wait();
}

void endLettuce() {
    commandList.Destroy();
    swapchain.Destroy();
    sync.Destroy();
    device.Destroy();
    instance.Destroy();
}

void mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        draw();
    }
}

void initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(width, height, "Lettuce ClearScreenSample", nullptr, nullptr);
}

void endWindow() {
    glfwDestroyWindow(window);
    glfwTerminate();
}