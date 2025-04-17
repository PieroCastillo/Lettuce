//
// Created by piero on 11/02/2024.
//
#pragma once
#ifdef VK_USE_PLATFORM_WIN32_KHR
#include <ddkernel.h>
#endif
#include <iostream>
#include <vector>
#include <cstdint>
#include <memory>
#include "IReleasable.hpp"
#include "GPU.hpp"
#include "Utils.hpp"
#include "Version.hpp"
#include "IManageHandle.hpp"

namespace Lettuce::Core
{
    class Instance : public IReleasable, public IManageHandle<VkInstance>
    {
    private:
        bool isSurfaceCreated = false;
        std::vector<char *> availableExtensionsNames;
        std::vector<char *> availableLayersNames;
        std::vector<const char *> requestedExtensionsNames;
        std::vector<const char *> requestedLayersNames;

        void listExtensions();
        void listLayers();

        void loadPlatformAndFeatures();

    public:
        VkInstance _instance;
        VkSurfaceKHR _surface;
        bool _debug;

        Instance(std::string appName, Version appVersion, std::vector<char *> requestedExtensions);
        void Release();
        template <typename T1, typename T2>
        void CreateSurface(T1 window, T2 process)
        {
            isSurfaceCreated = true;
            checkResult(CreateVkSurface(_instance, window, process, _surface, nullptr));
        }
        std::vector<GPU> getGPUs();

        bool IsSurfaceCreated();
    };
}