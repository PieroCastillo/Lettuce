//
// Created by piero on 11/02/2024.
//
module;
#include <algorithm>
#include <iostream>
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

export module Lettuce:GPU;

using namespace std;

export namespace Lettuce::Core
{
    class GPU
    {
    public:
        VkSurfaceKHR _surface;
        VkPhysicalDevice _pdevice;
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        VkPhysicalDeviceType deviceType;

        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
        VkSurfaceFormatKHR surfaceFormat;
        VkPresentModeKHR presentMode;

        std::string deviceName;
        bool geometryShaderPresent = false;
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        vector<char*> availableExtensionsNames;

        void Create(VkSurfaceKHR &surface, VkPhysicalDevice &device) {
            _surface = surface;
            _pdevice = device;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

            deviceType = deviceProperties.deviceType;
            geometryShaderPresent = deviceFeatures.geometryShader;
            deviceName = std::string(deviceProperties.deviceName);
            loadQueuesFamilies();
            checkSurfaceCapabilities();
            std::cout << "gpu ptr: " << _pdevice << std::endl;
        }

        bool GraphicsCapable() {
            return graphicsFamily.has_value() && presentFamily.has_value() && !formats.empty() && !presentModes.empty();;
        }

    private:

        void checkSurfaceCapabilities() {
            if(_surface == VK_NULL_HANDLE)
                return;

            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_pdevice, _surface, &capabilities);
            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(_pdevice, _surface, &formatCount, nullptr);

            if (formatCount != 0) {
                formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(_pdevice, _surface, &formatCount, formats.data());
            }

            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(_pdevice, _surface, &presentModeCount, nullptr);

            if (presentModeCount != 0) {
                presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(_pdevice, _surface, &presentModeCount, presentModes.data());
            }

            if(!GraphicsCapable())
                return;

            //selects the better format
            for (const auto& availableFormat : formats) {
                if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                   surfaceFormat = availableFormat;
                }
            }
            surfaceFormat = formats[0];

            //selects the presentations mode
            for (const auto& availablePresentMode : presentModes) {
                if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    presentMode = availablePresentMode;
                }
            }
            presentMode = VK_PRESENT_MODE_FIFO_KHR;
        }

        void loadQueuesFamilies() {
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(_pdevice, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(_pdevice, &queueFamilyCount, queueFamilies.data());

            int i = 0;
            for (const auto& queueFamily : queueFamilies) {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    graphicsFamily = i;
                }

                if(_surface != VK_NULL_HANDLE) {
                    VkBool32 presentSupport = false;
                    vkGetPhysicalDeviceSurfaceSupportKHR(_pdevice, i, _surface, &presentSupport);
                    if (presentSupport) {
                        presentFamily = i;
                    }
                }

                if(GraphicsCapable())
                    break;

                i++;
            }
        }


    };
}