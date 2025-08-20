#ifndef COMMON_HPP
#define COMMON_HPP

// standard headers
#include <cstdint>

#define NOMINMAX

#ifdef __INTELLISENSE__
#define WIN32_
#define __linux__
#endif

enum class LettuceResult
{
    Success,
    OutOfDeviceMemory,
    OutOfHostMemory,
    Unknown,
};

enum VkResult;

inline void handleResult(VkResult vkResult, LettuceResult& lResult)
{
    switch (vkResult)
    {
    case VK_SUCCESS:
        lResult = LettuceResult::Success;
        break;
    default:
        lResult = LettuceResult::Unknown;
        break;
    }
}

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
struct VkIndirectExecutionSet_T;

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
struct VkComponentMapping;
struct VkImageSubresourceRange;

#ifdef LETTUCE_EXPERIMENTAL

#endif

#endif

#endif // #define COMMON_HPP