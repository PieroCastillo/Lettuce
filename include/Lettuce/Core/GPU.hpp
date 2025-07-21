/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_GPU_HPP
#define LETTUCE_CORE_GPU_HPP 

#include "Common.hpp"
#include <optional>
#include <vector>

namespace Lettuce::Core
{
    class GPU
    {
    private:
        void checkSurfaceCapabilities();

        void loadQueuesFamilies();
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
        std::optional<uint32_t> computeFamily;
        std::vector<char *> availableExtensionsNames;
        uint32_t bufferImageGranularity = 0;

        GPU(VkPhysicalDevice &device);

        /// @brief Gets the supported Depth-Stencil Format
        /// Always return one value
        /// The value can be VK_FORMAT_D32_SFLOAT_S8_UINT or VK_FORMAT_D24_UNORM_S8_UINT
        /// vulkan docs: https://docs.vulkan.org/guide/latest/depth.html#depth-formats
        /// @return The supported format
        VkFormat GetDepthFormat();

        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    };
}
#endif // LETTUCE_CORE_GPU_HPP