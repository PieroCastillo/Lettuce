//
// Created by piero on 17/11/2024.
//
#pragma once
#include <string>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "Lettuce/Lettuce.Core.hpp"

/// @brief this is a simple class to implement Lettuce samples quickly
class LettuceSampleApp
{
protected:
    void initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    }
    void endWindow()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    void initializeLettuce()
    {
        instance._debug = true;
        instance.Create(appName, Lettuce::Core::Version{0, 1, 0, 0}, {});
        instance.CreateSurface(glfwGetWin32Window(window), GetModuleHandle(nullptr));
        auto gpus = instance.getGPUs();
        // create device

        features.MeshShading = false;
        features.ConditionalRendering = false;
        features.MemoryBudget = false;
        device.Create(instance, gpus.front(), features);
        swapchain.Create(device, width, height);
        createRenderPass();
        swapchain.SetResizeFunc([this]()
                                { return resizeCall();}, [this]()
                                { onResize(); });
    }

    virtual void createRenderPass()
    {
    }

    virtual void onResize()
    {
    }

    virtual void createObjects()
    {
    }

    virtual void updateData()
    {
    }

    virtual void draw()
    {
    }

    virtual void destroyObjects()
    {
    }

    virtual void beforeResize()
    {

    }

    void mainLoop()
    {
        while (!glfwWindowShouldClose(window))
        {
            std::cout << "-------------- frame ---------------" << std::endl;
            glfwPollEvents();
            updateData();
            draw();
        }
        device.Wait();
    }
    void destroyLettuce()
    {
        swapchain.Destroy();
        device.Destroy();
        instance.Destroy();
    }

    std::tuple<uint32_t, uint32_t> resizeCall()
    {
        int w, h;
        glfwGetWindowSize(window, &w, &h);
        width = w;
        height = h;
        beforeResize();

        return std::make_tuple((uint32_t)w, (uint32_t)h);
    }

public:
    GLFWwindow *window;
    int width = 800;
    int height = 600;
    /*Basic stuff for Lettuce usage*/
    std::string title = "Windows title";
    std::string appName = "Lettuce App";
    Lettuce::Core::Features features;
    Lettuce::Core::Instance instance;
    Lettuce::Core::Device device;
    Lettuce::Core::Swapchain swapchain;
    void run()
    {
        initWindow();
        initializeLettuce();
        createObjects();
        mainLoop();
        destroyObjects();
        destroyLettuce();
        endWindow();
    }
};