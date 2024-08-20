//
// Created by piero on 11/02/2024.
//
#pragma once
#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#include <ddkernel.h>
#elif defined(__linux__) || defined(__unix__)
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(__APPLE__)
#define VK_USE_PLATFORM_MACOS_MVK
#endif
#include <iostream>
#include <vector>
#include <cstdint>
#include <list>
#include <volk.h>
#include "GPU.hpp"
#include "Version.hpp"

namespace Lettuce::Core
{
    class Instance
    {
    private:
        bool isSurfaceCreated = false;
        std::vector<char *> availableExtensionsNames;
        std::vector<char *> availableLayersNames;
        std::vector<const char *> requestedExtensionsNames;
        std::vector<const char *> requestedLayersNames;
        VkDebugUtilsMessengerEXT debugMessenger;

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData);

        void listExtensions();
        void listLayers();

        void loadPlatformAndFeatures();

    public:
        VkInstance _instance;
        VkSurfaceKHR _surface;
        bool _debug = false;

        void Create(std::string appName, Version appVersion, std::vector<char *> requestedExtensions);
        template <typename T1, typename T2>
        void CreateSurface(T1 window, T2 process);
        std::list<GPU> getGPUs();

        bool IsSurfaceCreated();
        void Destroy();
    };
}