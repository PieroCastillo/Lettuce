//
// Created by piero on 15/02/2024.
//
module;
#include <iostream>
#include <stdexcept>
// #define VK_NO_PROTOTYPES
#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#include <ddkernel.h>
// #include <vulkan/vulkan_win32.h>
#endif
#if defined(__linux__) || defined(__unix__)
#define VK_USE_PLATFORM_XLIB_KHR
// #include <vulkan/vulkan_wayland.h>
#endif
#if defined(__APPLE__)
#define VK_USE_PLATFORM_MACOS_MVK
// #include <vulkan/vulkan_macos.h>
#endif
#define VOLK_IMPLEMENTATION
#include <volk.h>

export module Lettuce:Utils;

export namespace Lettuce::Core
{
    enum class SamplerAddressMode : uint32_t
    {
        Repeat = 0,
        MirroredRepeat = 1,
        ClampToEdge = 2,
        ClampToBorder = 3,
        // Provided by VK_VERSION_1_2, VK_KHR_sampler_mirror_clamp_to_edge
        MirrorClampToEdge = 4,
    };

    enum class FrontFace : uint32_t
    {
        CounterClockwise = 0,
        Clockwise = 1,
    };

    enum class DescriptorType
    {
        Sampler = 0,
        CombinedImageSampler = 1,
        SampledImage = 2,
        StorageImage = 3,
        UniformTexelBuffer = 4,
        StorageTexelBuffer = 5,
        UniformBuffer = 6,
        StorageBuffer = 7,
        UniformBufferDynamic = 8,
        StorageBufferDynamic = 9,
        InputAttachment = 10,
        // Provided by VK_VERSION_1_3
        InlineUniformBlock = 1000138000,
        // Provided by VK_KHR_acceleration_structure
        AccelerationStructure = 1000150000,
        Mutable = 1000351000,
    };

    enum class PipelineStage : uint32_t
    {
        Vertex = 0x00000001,
        TessellationControl = 0x00000002,
        TessellationEvaluation = 0x00000004,
        Geometry = 0x00000008,
        Fragment = 0x00000010,
        Compute = 0x00000020,
        AllGraphics = 0x0000001F,
        All = 0x7FFFFFFF,
        Task = 0x00000040,
        Mesh = 0x00000080,
    };

    enum class Format32
    {
        Vec1F = 100, // VK_FORMAT_R32_SFLOAT = 100,
        Vec2F = 103, // VK_FORMAT_R32G32_SFLOAT = 103,
        Vec3F = 106, // VK_FORMAT_R32G32B32_SFLOAT = 106,
        Vec4F = 109, // VK_FORMAT_R32G32B32A32_SFLOAT = 109,
        Vec1I = 99,  // VK_FORMAT_R32_SINT = 99,
        Vec2I = 102, // VK_FORMAT_R32G32_SINT = 102,
        Vec3I = 105, // VK_FORMAT_R32G32B32_SINT = 105,
        Vec4I = 108, // VK_FORMAT_R32G32B32A32_SINT = 108,
        Vec1U = 98,  // VK_FORMAT_R32_UINT = 98,
        Vec2U = 101, // VK_FORMAT_R32G32_UINT = 101,
        Vec3U = 104, // VK_FORMAT_R32G32B32_UINT = 104,
        Vec4U = 103, // VK_FORMAT_R32G32B32A32_UINT = 107,
    };

    enum class Format64
    {
        Vec1F = 112, // VK_FORMAT_R64_SFLOAT
        Vec2F = 115, // VK_FORMAT_R64G64_SFLOAT
        Vec3F = 118, // VK_FORMAT_R64G64B64_SFLOAT
        Vec4F = 121, // VK_FORMAT_R64G64B64A64_SFLOAT
        Vec1I = 111, // VK_FORMAT_R64_SINT
        Vec2I = 114, // VK_FORMAT_R64G64_SINT
        Vec3I = 117, // VK_FORMAT_R64G64B64_SINT
        Vec4I = 120, // VK_FORMAT_R64G64B64A64_SINT
        Vec1U = 110, // VK_FORMAT_R64_UINT
        Vec2U = 113, // VK_FORMAT_R64G64_UINT
        Vec3U = 116, // VK_FORMAT_R64G64B64_UINT
        Vec4U = 119, // VK_FORMAT_R64G64B64A64_UINT
    };

    void checkResult(const VkResult result, std::string onSucessMessage = "created successfully!")
    {
        if (result == VK_SUCCESS)
        {
            // std::cout << onSucessMessage << std::endl;
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create vk object");
        }
    }

    char *GetSurfaceExtensionNameByPlatform()
    {
        char *surfaceExtensionName;
#if defined(_WIN32)
        return (char *)VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#endif
#if defined(__linux__)
        return (char *)VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
#endif
#if defined(__APPLE__)
        return (char *)VK_MVK_MACOS_SURFACE_EXTENSION_NAME;
#endif
        return (char *)"";
    }

    template <typename T1, typename T2>
    VkResult CreateVkSurface(VkInstance instance, T1 window, T2 process, VkSurfaceKHR &surface, const VkAllocationCallbacks *allocator)
    {
#if defined(_WIN32)
        VkWin32SurfaceCreateInfoKHR surfaceCI = {
            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .hinstance = (static_cast<HINSTANCE>(process)),
            .hwnd = (static_cast<HWND>(window))};
        return vkCreateWin32SurfaceKHR(instance, &surfaceCI, allocator, &surface);
#endif
#if defined(__linux__)
        return VK_ERROR_INITIALIZATION_FAILED;
#endif
#if defined(__APPLE__)
        return VK_ERROR_INITIALIZATION_FAILED;
#endif
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }
}