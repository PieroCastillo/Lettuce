//
// Created by piero on 17/11/2024.
//
#pragma once
#include <string>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <iomanip>
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
        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, keyCallback);
        glfwSetCursorPosCallback(window, cursorCallback);
        //if (glfwRawMouseMotionSupported())
        //    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
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
                                { return resizeCall(); }, [this]()
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

    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        LettuceSampleApp *app = static_cast<LettuceSampleApp *>(glfwGetWindowUserPointer(window));
        if (app)
        {
            app->onKeyPress(key, action, mods);
        }
    }

    // modifies position on XY of camera
    // <- -x
    // -> +x
    // up +y
    // down -y
    void onKeyPress(int key, int action, int mods)
    {
        double dx, dy, dz;
        dx = 0.3;
        dy = 0.3;
        dz = 0.3;
        std::cout << std::fixed << std::setprecision(6);
        if (key == GLFW_KEY_UP && action == GLFW_REPEAT)
        {
            cameraPosition.y += dy;
            std::cout << "camera pos: " << cameraPosition.x << cameraPosition.y << cameraPosition.z  << std::endl;
        }
        else if (key == GLFW_KEY_DOWN && action == GLFW_REPEAT)
        {
            cameraPosition.y -= dy;
        }
        else if (key == GLFW_KEY_LEFT && action == GLFW_REPEAT)
        {
            cameraPosition.z -= dz;
        }
        else if (key == GLFW_KEY_RIGHT && action == GLFW_REPEAT)
        {
            cameraPosition.z += dz;
        }

        if(action == GLFW_REPEAT)
            std::cout << "camera pos: x : " << cameraPosition.x << " y : " <<  cameraPosition.y << " z : " << cameraPosition.z  << std::endl;
    }

    static void cursorCallback(GLFWwindow *window, double xpos, double ypos)
    {
        LettuceSampleApp *app = static_cast<LettuceSampleApp *>(glfwGetWindowUserPointer(window));
        if (app)
        {
            app->onCursorMotion(xpos, ypos);
        }
    }

    // up -> +z
    // down -> -z
    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
    {
        LettuceSampleApp *app = static_cast<LettuceSampleApp *>(glfwGetWindowUserPointer(window));
        if (app)
        {
            app->onMouseButtonCore(button, action, mods);
        }
    }

    virtual void onCursorMotion(double xpos, double ypos)
    {
        
    }
    bool pressed = false;
    double xl, yl;
    // modifies direction of camera
    void onMouseButtonCore(int button, int action, int mods)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            pressed = true;
            glfwGetCursorPos(window, &xl, &yl);
        }
        else{
            pressed = false;
        }
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
    /*Variable to travel around the space*/
    glm::vec3 cameraPosition = {20, 20, 30};
    glm::vec3 cameraDirection = {0, 0, 0};
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