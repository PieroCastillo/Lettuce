//
// Created by piero on 14/02/2024.
//
#pragma once
#include <iostream>
#include <set>
#include <vector>
#include <volk.h>
#define VMA_IMPLEMENTATION  
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
#include <vma/vk_mem_alloc.h>
#include "Lettuce/Core/Instance.hpp"
#include "Lettuce/Core/GPU.hpp"

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
        VkQueue _graphicsQueue;
        VkQueue _presentQueue;
        VmaAllocator allocator;
        GPU _gpu;

        void Create(Instance &instance, GPU &gpu, std::vector<char *> requestedExtensions);

        void Wait();

        void Destroy();
    };
}