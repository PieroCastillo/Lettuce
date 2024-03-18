//
// Created by piero on 11/02/2024.
// instance.ixx : co
//
module;

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#include <ddkernel.h>
#endif
#include <iostream>
#include <vector>
#include <cstdint>
#include <list>
#include <vulkan/vulkan.h>

export module Lettuce:Instance;

import :GPU;
import :Version;
import :Utils;
import :Loader;

using namespace std;

export namespace Lettuce::Core
{
    class Instance
    {
    private:
        bool isSurfaceCreated = false;
        vector<char*> availableExtensionsNames;
        vector<char*> availableLayersNames;
        vector<const char*> requestedExtensionsNames;
        std::vector<const char*> requestedLayersNames;
        VkDebugUtilsMessengerEXT debugMessenger;

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData) {

            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

            return VK_FALSE;
        }

        void listExtensions() {
            uint32_t availableExtensionCount = 0;
            vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
            vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
            vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());
            for(auto ext : availableExtensions) {
                availableExtensionsNames.push_back(ext.extensionName);
                cout << "available instance extensions :" << ext.extensionName << endl;
            }
        }

        void listLayers() {
            uint32_t availableLayerCount = 0;
            vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);
            vector<VkLayerProperties> availableLayers(availableLayerCount);
            vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.data());
            for(auto layer : availableLayers) {
                availableLayersNames.push_back(layer.layerName);
                cout << "available instance layer : " << layer.layerName << endl;
            }
        }

        void loadPlatformAndFeatures() {
            auto name = GetSurfaceExtensionNameByPlatform();
            std::cout << "surface ext name: " << name << std::endl;
            requestedExtensionsNames.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
            if(_debug) {
                requestedExtensionsNames.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }
            requestedExtensionsNames.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            requestedExtensionsNames.emplace_back(name);
        }

    public:
        VkInstance _instance;
        VkSurfaceKHR _surface;
        bool _debug = false;

        void Create(string appName, Version appVersion, std::vector<char*> requestedExtensions) {
            for(auto ext : requestedExtensions) {
                requestedExtensionsNames.emplace_back(ext);
            }
            if(_debug) {
                requestedLayersNames.emplace_back("VK_LAYER_KHRONOS_validation");
            }

            Lettuce::Core::VulkanFunctions vf;
            auto loaded = vf.load_vulkan_library();

            if(!loaded) {
                std::cout << "cannot load vulkan library" << std::endl;
                return;
            }
            VkDebugUtilsMessengerCreateInfoEXT debugUtilsCI =
            {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                .pfnUserCallback = debugCallback,
                .pUserData = nullptr, // Optional
            };

            VkApplicationInfo appI = {
                .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pApplicationName = appName.c_str(),
                .applicationVersion =  VK_MAKE_API_VERSION(appVersion.variant, appVersion.major, appVersion.minor, appVersion.patch),
                .pEngineName = "Lettuce Rendering Engine",
                .engineVersion = VK_MAKE_API_VERSION(VARIANT_VERSION, MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION),
                .apiVersion = VK_VERSION_1_3
            };
            listExtensions();
            loadPlatformAndFeatures();
            listLayers();

            VkInstanceCreateInfo instanceCI = {
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pApplicationInfo = &appI,
                .enabledLayerCount = (uint32_t)requestedLayersNames.size(),
                .ppEnabledLayerNames = requestedLayersNames.data(),
                .enabledExtensionCount = (uint32_t)requestedExtensionsNames.size(),
                .ppEnabledExtensionNames = requestedExtensionsNames.data()
            };

            if(_debug){
                instanceCI.pNext = &debugUtilsCI;
            }

            checkResult(vkCreateInstance(&instanceCI, nullptr, &_instance),"instance created successfully");
            if(_debug) {
                checkResult(CreateDebugUtilsMessengerEXT(_instance, &debugUtilsCI, nullptr, &debugMessenger), "debug messenger created successfully");
            }
        }

        template<typename T1, typename  T2>
        void CreateSurface(T1 window, T2 process) {
            isSurfaceCreated = true;
            checkResult(CreateVkSurface(_instance,window,process,_surface,nullptr), "surface created successfully");
            //std::cout << CreateVkSurface(_instance,window,process,_surface,nullptr) << std::endl;
        }

        std::list<GPU> getGPUs(){
            uint32_t physicalDevicesCount = 0;
            vkEnumeratePhysicalDevices(_instance, &physicalDevicesCount, nullptr);
            std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
            vkEnumeratePhysicalDevices(_instance, &physicalDevicesCount, physicalDevices.data());

            list<GPU> gpus(physicalDevicesCount-1);
            for(auto dev : physicalDevices) {
                std::cout << "before pass ptr: " << dev << std::endl;
                GPU gpu;
                gpu.Create(_surface, dev);
                gpus.push_front(gpu);
                //std::cout << "in function gpu ptr: " << gpus.front()._pdevice << std::endl;
            }
            return gpus;
        }

        bool IsSurfaceCreated() {
            return isSurfaceCreated;
        }

        void Destroy()
        {
            if(isSurfaceCreated) {
                vkDestroySurfaceKHR(_instance, _surface, nullptr);
            }
            if(_debug) {
                DestroyDebugUtilsMessengerEXT(_instance, debugMessenger, nullptr);
            }
            vkDestroyInstance(_instance, nullptr);
        }
    };
}