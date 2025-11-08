#ifndef COMMON_HPP
#define COMMON_HPP

// standard headers
#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
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
        Staging,                 // TRANSFER_SRC
        ShaderGeometry,          // VERTEX | INDEX | TRANSFER_DST
        ShaderReadOnlyResource,  // UNIFORM | SHADER_DEVICE_ADDRESS
        ShaderReadWriteResource, // STORAGE | SHADER_DEVICE_ADDRESS
        Descriptors,             // RESOURCE_DESCRIPTOR_BUFFER | SAMPLER_DESCRIPTOR_BUFFER
        Indirect,                // INDIRECT_BUFFER | SHADER_DEVICE_ADDRESS
    };

    enum class MemoryAccess
    {
        FastGPUReadWrite,    // DEVICE_LOCAL
        FastCPUWriteGPURead, // DEVICE_LOCAL+HOST_VISIBLE+HOST_COHERENT
        FastGPUWriteCPURead, // HOST_VISIBLE+HOST_COHERENT
        FastCPUReadWrite,    // HOST_VISIBLE+HOST_COHERENT+HOST_CACHED
    };

    enum class LettuceResult
    {
        Success,
        OutOfDeviceMemory,
        OutOfHostMemory,
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
        NotReady,
        NotFound,
        Unknown,
    };

    [[nodiscard]] constexpr VkBufferUsageFlags mapAllocatorUsageToVk(AllocatorUsage usage) noexcept
    {
        using enum AllocatorUsage;
        constexpr std::array<VkBufferUsageFlags, 6> table{
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT,
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        };

        return table[std::to_underlying(usage)];
    }

    uint32_t findMemoryTypeIndex(VkDevice device, VkPhysicalDevice gpu, uint32_t typeFilter, MemoryAccess access);

    template <typename First, typename... Rest>
    bool haveSameSize(const First& first, const Rest&... rest)
    {
        size_t size = first.size();
        return ((rest.size() == size) && ...);
    }

    class LettuceException : public std::runtime_error {
    public:
        explicit LettuceException(LettuceResult r)
            : std::runtime_error(resultToString(r)), result(r) {}

        explicit LettuceException(VkResult r)
            : std::runtime_error(resultToString(r)), vkResult(r) {}

        LettuceResult result;
        VkResult vkResult;

    private:
        static std::string resultToString(LettuceResult r) {
            switch (r) {
            case LettuceResult::OutOfDeviceMemory: return "Out of Device Memory";
            case LettuceResult::OutOfHostMemory: return "Out of Host Memory";
            default: return "Unknown error";
            }
        }

        static std::string resultToString(VkResult r) {
            switch (r) {
                // VK_ERROR_OUT_OF_DEVICE_MEMORY
            case -2: return "Out of Device Memory";
                // VK_ERROR_OUT_OF_HOST_MEMORY
            case -1: return "Out of Host Memory";
            default: return "Unknown error";
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

    using AllocationHandle = uint32_t;

    struct BufferAllocation
    {
        VkBuffer buffer;
        uint32_t size;
        uint32_t offset;
        void* data;
        AllocationHandle handle;
    };

    struct ImageAllocation
    {
        VkImage image;
        AllocationHandle handle;
    };

    struct BufferHandle
    {
        VkDeviceAddress address;
        uint32_t size;
    };

    struct TextureHandle
    {
        VkImageView view;
        VkImageLayout layout;
    };

    class IDevice
    {
    public:
        VkDevice m_device;
        VkInstance m_instance;
        VkPhysicalDevice m_physicalDevice;
        VkQueue m_graphicsQueue;
        VkQueue m_computeQueue;
        VkQueue m_transferQueue;
        uint32_t m_graphicsQueueFamilyIndex;
        uint32_t m_computeQueueFamilyIndex;
        uint32_t m_transferQueueFamilyIndex;
        bool supportBufferUsage2;
    };
    
    struct DescriptorBindingInfo
    {
        std::string bindingName;
        uint32_t bindingIdx;
        VkDescriptorType type;
        uint32_t count;
    };

    struct DescriptorSetLayoutInfo
    {
        std::string setName;
        uint32_t setIdx;
        std::vector<DescriptorBindingInfo> bindings;
    };
}
#endif // COMMON_HPP