#ifndef COMMON_HPP
#define COMMON_HPP

// standard headers
#include <array>
#include <cstdint>
#include <iostream>
#include <print>
#include <stacktrace>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

// library headers
#include <volk.h>

#define NOMINMAX

#ifdef __INTELLISENSE__
#define WIN32_
#define __linux__
#endif

enum VkResult;

template<typename T>
struct VkHandle { using type = T*; };

#ifndef VULKAN_H_

struct VkInstance_T;
struct VkPhysicalDevice_T;
struct VkDevice_T;
struct VkQueue_T;
struct VkSemaphore_T;
struct VkCommandBuffer_T;
struct VkFence_T;
struct VkDeviceMemory_T;
struct VkBuffer_T;
struct VkImage_T;
struct VkEvent_T;
struct VkImageView_T;
struct VkShaderModule_T;
struct VkPipelineCache_T;
struct VkPipelineBinaryKHR_T;
struct VkPipelineLayout_T;
struct VkPipeline_T;
struct VkDescriptorSetLayout_T;
struct VkSampler_T;
struct VkCommandPool_T;
struct VkSurfaceKHR_T;
struct VkSwapchainKHR_T;
struct VkVideoSessionKHR_T;
struct VkVideoSessionParametersKHR_T;
struct VkAccelerationStructureKHR_T;
struct VkDeferredOperationKHR_T;
struct VkIndirectCommandsLayoutEXT_T;
struct VkIndirectExecutionSetEXT_T;
struct VmaAllocation_T;
struct VmaAllocator_T;

using VkInstance = VkHandle<VkInstance_T>::type;
using VkPhysicalDevice = VkHandle<VkPhysicalDevice_T>::type;
using VkDevice = VkHandle<VkDevice_T>::type;
using VkQueue = VkHandle<VkQueue_T>::type;
using VkSemaphore = VkHandle<VkSemaphore_T>::type;
using VkCommandBuffer = VkHandle<VkCommandBuffer_T>::type;
using VkFence = VkHandle<VkFence_T>::type;
using VkDeviceMemory = VkHandle<VkDeviceMemory_T>::type;
using VkBuffer = VkHandle<VkBuffer_T>::type;
using VkImage = VkHandle<VkImage_T>::type;
using VkEvent = VkHandle<VkEvent_T>::type;
using VkImageView = VkHandle<VkImageView_T>::type;
using VkShaderModule = VkHandle<VkShaderModule_T>::type;
using VkPipelineCache = VkHandle<VkPipelineCache_T>::type;
using VkPipelineBinaryKHR = VkHandle<VkPipelineBinaryKHR_T>::type;
using VkPipelineLayout = VkHandle<VkPipelineLayout_T>::type;
using VkPipeline = VkHandle<VkPipeline_T>::type;
using VkDescriptorSetLayout = VkHandle<VkDescriptorSetLayout_T>::type;
using VkSampler = VkHandle<VkSampler_T>::type;
using VkCommandPool = VkHandle<VkCommandPool_T>::type;
using VkSurfaceKHR = VkHandle<VkSurfaceKHR_T>::type;
using VkSwapchainKHR = VkHandle<VkSwapchainKHR_T>::type;
using VkVideoSessionKHR = VkHandle<VkVideoSessionKHR_T>::type;
using VkVideoSessionParametersKHR = VkHandle<VkVideoSessionParametersKHR_T>::type;
using VkAccelerationStructureKHR = VkHandle<VkAccelerationStructureKHR_T>::type;
using VkDeferredOperationKHR = VkHandle<VkDeferredOperationKHR_T>::type;
using VkIndirectCommandsLayoutEXT = VkHandle<VkIndirectCommandsLayoutEXT_T>::type;
using VkIndirectExecutionSetEXT = VkHandle<VkIndirectExecutionSetEXT_T>::type;
using VmaAllocation = VkHandle<VmaAllocation_T>::type;
using VmaAllocator = VkHandle<VmaAllocator_T>::type;

using VkDeviceSize = std::uint64_t;
using VkFlags = std::uint32_t;
using VkShaderStageFlags = VkFlags;
using VkImageUsageFlags = VkFlags;

enum VkFormat;
enum VkImageLayout;
enum VkFilter;
enum VkSamplerAddressMode;
enum VkBorderColor;
enum VkCompareOp;
enum VkImageType;
enum VkDescriptorType;
enum VkPrimitiveTopology;
struct VkComponentMapping;
struct VkImageSubresourceRange;

#endif // VULKAN_H_

namespace Lettuce::Core
{
    enum class AllocatorUsage
    {
        General,                 // TRANSFER_SRC | TRANSFER_DST | SHADER_DEVICE_ADDRESS
        ShaderReadOnlyResource,  // UNIFORM | TRANSFER_SRC | TRANSFER_DST | SHADER_DEVICE_ADDRESS
        ShaderReadWriteResource, // STORAGE | TRANSFER_SRC | TRANSFER_DST | SHADER_DEVICE_ADDRESS
        Descriptors,             // RESOURCE_DESCRIPTOR_BUFFER | SAMPLER_DESCRIPTOR_BUFFER | SHADER_DEVICE_ADDRESS
        Indirect,                // INDIRECT_BUFFER | SHADER_DEVICE_ADDRESS
    };

    enum class MemoryAccess
    {
        GPUOnly, // DEVICE_LOCAL
        Shared,  // DEVICE_LOCAL+HOST_VISIBLE+HOST_COHERENT
    };

    enum class LettuceResult
    {
        Success,
        OutOfDeviceMemory,
        OutOfHostMemory,
        OutOfBounds,
        AllocationFailed,
        DoubleFree,
        InvalidDevice,
        InvalidOperation,
        InvalidShaderEntryPoint,
        InitializationFailed,
        RequiredMemoryNotFound,
        ShaderReflectionFailed,
        ShaderCompilationFailed,
        ShaderParametersMismatch,
        TypeMismatch,
        NotReady,
        NotFound,
        Unknown,
    };

    uint32_t findMemoryTypeIndex(VkDevice device, VkPhysicalDevice gpu, uint32_t typeFilter, MemoryAccess access);

    template <typename First, typename... Rest>
    bool haveSameSize(const First& first, const Rest&... rest)
    {
        size_t size = first.size();
        return ((rest.size() == size) && ...);
    }

    class NotImplemented : public std::logic_error  {
    public:
        explicit NotImplemented(std::string msg) : std::logic_error(msg) {}
    };

    class LettuceException : public std::runtime_error {
    public:
        explicit LettuceException(LettuceResult r)
            : std::runtime_error(resultToString(r)), result(r) { printStacktrace(); }

        explicit LettuceException(VkResult r)
            : std::runtime_error(resultToString(r)), vkResult(r) { printStacktrace(); }

        LettuceResult result;
        VkResult vkResult;

    private:
        void printStacktrace() const
        {
            std::println("[Lettuce Exception]");
            std::println(std::cerr, "Error: {}", what());
            std::println(std::cerr, "Stack trace:\n{}", std::stacktrace::current());
        }
        static std::string resultToString(LettuceResult r) {
            switch (r) {
            case LettuceResult::Success: return "Success";
            case LettuceResult::OutOfDeviceMemory: return "Out of Device Memory";
            case LettuceResult::OutOfHostMemory: return "Out of Host Memory";
            case LettuceResult::OutOfBounds: return "Out of Bounds";
            case LettuceResult::AllocationFailed: return "Allocation Failed";
            case LettuceResult::DoubleFree: return "Double Free Detected";
            case LettuceResult::InvalidDevice: return "Invalid Device";
            case LettuceResult::InvalidOperation: return "Invalid Operation";
            case LettuceResult::InvalidShaderEntryPoint: return "Invalid Shader Entry Point";
            case LettuceResult::InitializationFailed: return "Initialization Failed";
            case LettuceResult::RequiredMemoryNotFound: return "Required Memory Not Found";
            case LettuceResult::ShaderReflectionFailed: return "Shader Reflection Failed";
            case LettuceResult::ShaderCompilationFailed: return "Shader Compilation Failed";
            case LettuceResult::ShaderParametersMismatch: return "Shader Parameters Mismatch";
            case LettuceResult::TypeMismatch: return "Type Mismatch";
            case LettuceResult::NotReady: return "Not Ready";
            case LettuceResult::NotFound: return "Not Found";
            case LettuceResult::Unknown: return "Unknown Error";
            }
        }

        static std::string resultToString(VkResult r)
        {
            switch (r)
            {
            case 1:
                return "VK_NOT_READY";
            case 2:
                return "VK_TIMEOUT";
            case 3:
                return "VK_EVENT_SET";
            case 4:
                return "VK_EVENT_RESET";
            case 5:
                return "VK_INCOMPLETE";

            case -1:
                return "VK_ERROR_OUT_OF_HOST_MEMORY";
            case -2:
                return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
            case -3:
                return "VK_ERROR_INITIALIZATION_FAILED";
            case -4:
                return "VK_ERROR_DEVICE_LOST";
            case -5:
                return "VK_ERROR_MEMORY_MAP_FAILED";
            case -6:
                return "VK_ERROR_LAYER_NOT_PRESENT";
            case -7:
                return "VK_ERROR_EXTENSION_NOT_PRESENT";
            case -8:
                return "VK_ERROR_FEATURE_NOT_PRESENT";
            case -9:
                return "VK_ERROR_INCOMPATIBLE_DRIVER";
            case -10:
                return "VK_ERROR_TOO_MANY_OBJECTS";
            case -11:
                return "VK_ERROR_FORMAT_NOT_SUPPORTED";
            case -12:
                return "VK_ERROR_FRAGMENTED_POOL";
            case -13:
                return "VK_ERROR_UNKNOWN";

            case -1000011001:
                return "VK_ERROR_VALIDATION_FAILED";
            case -1000069000:
                return "VK_ERROR_OUT_OF_POOL_MEMORY";
            case -1000072003:
                return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
            case -1000257000:
                return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
            case -1000161000:
                return "VK_ERROR_FRAGMENTATION";

            case 1000297000:
                return "VK_PIPELINE_COMPILE_REQUIRED";
            case -1000174001:
                return "VK_ERROR_NOT_PERMITTED";

            case -1000000000:
                return "VK_ERROR_SURFACE_LOST_KHR";
            case -1000000001:
                return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
            case 1000001003:
                return "VK_SUBOPTIMAL_KHR";
            case -1000001004:
                return "VK_ERROR_OUT_OF_DATE_KHR";
            case -1000003001:
                return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";

            case -1000012000:
                return "VK_ERROR_INVALID_SHADER_NV";

            case -1000023000:
                return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
            case -1000023001:
                return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
            case -1000023002:
                return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
            case -1000023003:
                return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
            case -1000023004:
                return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
            case -1000023005:
                return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";

            case -1000158000:
                return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
            case -1000208000:
                return "VK_ERROR_PRESENT_TIMING_QUEUE_FULL_EXT";
            case -1000255000:
                return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";

            case 1000268000:
                return "VK_THREAD_IDLE_KHR";
            case 1000268001:
                return "VK_THREAD_DONE_KHR";
            case 1000268002:
                return "VK_OPERATION_DEFERRED_KHR";
            case 1000268003:
                return "VK_OPERATION_NOT_DEFERRED_KHR";

            case -1000299000:
                return "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR";
            case -1000338000:
                return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";

            case 1000482000:
                return "VK_INCOMPATIBLE_SHADER_BINARY_EXT";
            case 1000483000:
                return "VK_PIPELINE_BINARY_MISSING_KHR";
            case -1000483000:
                return "VK_ERROR_NOT_ENOUGH_SPACE_KHR";

            default:
                return "Unknown VkResult (" + std::to_string(static_cast<int32_t>(r)) + ")";
            }
        }
    };

    /// @brief throws an LettuceException if vkResult is not VK_SUCCESS
    /// This method MUST NOT be called in performance critical paths
    /// @param vkResult the VkResult to check
    inline void handleResult(VkResult vkResult)
    {
        // VK_SUCCESS
        if (vkResult != 0) [[unlikely]]
        {
            throw LettuceException(vkResult);
        }
    }

    constexpr uint32_t align_up(uint32_t data, uint32_t alignment) {
        if ((alignment & (alignment - 1)) != 0)
            throw "alignment must be a power of two";

        return (data + alignment - 1) & ~(alignment - 1);
    }

    template <typename T, typename... Members>
    auto unpack(const std::vector<T>& src, Members... members) {
        std::tuple<std::vector<std::decay_t<decltype(src[0].*members)>>...> result;

        (..., std::get<std::vector<std::decay_t<decltype(src[0].*members)>>>(result)
            .reserve(src.size()));

        for (const T& obj : src) {
            (..., std::get<std::vector<std::decay_t<decltype(obj.*members)>>>(result)
                .push_back(obj.*members));
        }

        return result;
    }
}
#endif // COMMON_HPP