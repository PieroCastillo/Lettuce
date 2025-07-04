//
// Created by piero on 17/11/2024.
//
#pragma once
#include <string>
#include <iomanip>
#include <memory>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include "Lettuce/Lettuce.Core.hpp"
#include "Lettuce/Lettuce.X3D.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::X3D;

/// @brief this is a simple class to implement Lettuce samples quickly
class LettuceSampleApp
{
protected:
    void initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, keyCallback);
        glfwSetCursorPosCallback(window, cursorCallback);
        // if (glfwRawMouseMotionSupported())
        //     glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
    }
    void endWindow()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void initializeLettuce()
    {
        Lettuce::Core::Version version = {0, 1, 0, 0};
        std::vector<char *> reqExts = {};
        instance = std::make_shared<Lettuce::Core::Instance>(appName, version, reqExts);
        instance->CreateSurface(glfwGetWin32Window(window), GetModuleHandle(nullptr));
        
        auto gpus = instance->getGPUs();

        // create device
        features.MeshShading = false;
        device = std::make_shared<Lettuce::Core::Device>(instance, gpus.front(), features);
        swapchain = std::make_shared<Lettuce::Core::Swapchain>(device, width, height);

        releaseQueue.Push(instance);
        releaseQueue.Push(device);
        releaseQueue.Push(swapchain);

        createRenderPass();
        swapchain->SetResizeFunc([this]()
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
        glfwShowWindow(window);
        while (!glfwWindowShouldClose(window))
        {
            std::cout << "-------------- frame ---------------" << std::endl;
            glfwPollEvents();
            updateData();
            draw();
        }
        device->Wait();
    }
    void destroyLettuce()
    {
        releaseQueue.ReleaseAll();
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

    const double fixedDt = 0.01;
    void onKeyPress(int key, int action, int mods)
    {
        // float su, sd, sdt; // sensibility of up, direction, direction orthogonal vectors
        // su = 0.05;
        // sd = 0.05;
        // sdt = 0.05;
        // std::cout << std::fixed << std::setprecision(6);
        // auto distance = camera.center - camera.eye;
        // auto dd = -sd * distance; // the minus is for thew reflect effect
        // auto ddt = -sdt * (glm::cross(camera.up, distance));
        // auto du = su * camera.up;
        // forward
        if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            // camera.center += dd;
            // camera.eye += dd;
            
            camera.ProcessKeyboard(CameraMovement::Forward, fixedDt);
        }
        // backward
        else if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            // camera.center -= dd;
            // camera.eye -= dd;
            camera.ProcessKeyboard(CameraMovement::Backward, fixedDt);
        }
        // left
        else if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            // camera.center -= ddt;
            // camera.eye -= ddt;
            camera.ProcessKeyboard(CameraMovement::Left, fixedDt);
        }
        // right
        else if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            // camera.center += ddt;
            // camera.eye += ddt;
            camera.ProcessKeyboard(CameraMovement::Right, fixedDt);
        }
        // up
        else if (key == GLFW_KEY_R && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            // camera.center += du;
            // camera.eye += du;
        }
        // down
        else if (key == GLFW_KEY_F && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            // camera.center -= du;
            // camera.eye -= du;
        }
        // camera.Update();

        // if (action == GLFW_REPEAT)
        //     std::cout << "camera pos: x : " << cameraPosition.x << " y : " << cameraPosition.y << " z : " << cameraPosition.z << std::endl;
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

    // double yaw = 0;
    // double pitch = 0;
    // double roll = 0;

    virtual void onCursorMotion(double xpos, double ypos)
    {
        double sx = 0.0001f, sy = 0.0001f; /*sensiblities*/
        if (pressed)
        {
            double dx = xl - xpos;
            double dy = - yl + ypos;
            // camera.Rotate(sx * dx, sy * dy);
            camera.ProcessMouseMovement(dx,dy);
        }
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
            // camera.ProcessMouseScroll()
        }
        else
        {
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
    Lettuce::Core::ReleaseQueue releaseQueue;
    std::shared_ptr<Lettuce::Core::Instance> instance;
    std::shared_ptr<Lettuce::Core::Device> device;
    std::shared_ptr<Lettuce::Core::Swapchain> swapchain;
    Lettuce::X3D::Camera3D camera;
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