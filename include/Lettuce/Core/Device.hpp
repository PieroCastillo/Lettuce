//
// Created by piero on 14/02/2024.
//
#pragma once

#include <iostream>
#include <set>
#include <vector>
#include <volk.h>
#include "VmaUsage.hpp"
#include "Instance.hpp"
#include "GPU.hpp"

namespace Lettuce::Core
{
    class Device
    {
    private:
        std::vector<char *> availableExtensionsNames;
        std::vector<char *> availableLayersNames;
        std::vector<const char *> requestedExtensionsNames;
        std::vector<const char *> requestedLayersNames;

        void listExtensions();

        void listLayers();

        void loadExtensionsLayersAndFeatures();

    public:
        Instance _instance;
        VkPhysicalDevice _pdevice;
        VkDevice _device;
        std::vector<VkQueue> _graphicsQueues;
        VkQueue _presentQueue;
        VmaAllocator allocator;
        GPU _gpu;

        void Create(Instance &instance, GPU &gpu, std::vector<char *> requestedExtensions, uint32_t graphicsQueuesCount = 1);

        void Wait();

        void Destroy();
    };
}