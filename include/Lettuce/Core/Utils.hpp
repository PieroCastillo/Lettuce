//
// Created by piero on 15/02/2024.
//
#pragma once
#include <iostream>
#include <stdexcept>
#include <vector>
#include <list>
#include <functional>
#include <type_traits>

// credits: https://stackoverflow.com/a/69183821/13766341
#define MAKE_ENUM_FLAGS(TEnum)                                                      \
    inline TEnum operator~(TEnum a)                                                 \
    {                                                                               \
        using TUnder = typename std::underlying_type_t<TEnum>;                      \
        return static_cast<TEnum>(~static_cast<TUnder>(a));                         \
    }                                                                               \
    inline TEnum operator|(TEnum a, TEnum b)                                        \
    {                                                                               \
        using TUnder = typename std::underlying_type_t<TEnum>;                      \
        return static_cast<TEnum>(static_cast<TUnder>(a) | static_cast<TUnder>(b)); \
    }                                                                               \
    inline TEnum operator&(TEnum a, TEnum b)                                        \
    {                                                                               \
        using TUnder = typename std::underlying_type_t<TEnum>;                      \
        return static_cast<TEnum>(static_cast<TUnder>(a) & static_cast<TUnder>(b)); \
    }                                                                               \
    inline TEnum operator^(TEnum a, TEnum b)                                        \
    {                                                                               \
        using TUnder = typename std::underlying_type_t<TEnum>;                      \
        return static_cast<TEnum>(static_cast<TUnder>(a) ^ static_cast<TUnder>(b)); \
    }                                                                               \
    inline TEnum &operator|=(TEnum &a, TEnum b)                                     \
    {                                                                               \
        using TUnder = typename std::underlying_type_t<TEnum>;                      \
        a = static_cast<TEnum>(static_cast<TUnder>(a) | static_cast<TUnder>(b));    \
        return a;                                                                   \
    }                                                                               \
    inline TEnum &operator&=(TEnum &a, TEnum b)                                     \
    {                                                                               \
        using TUnder = typename std::underlying_type_t<TEnum>;                      \
        a = static_cast<TEnum>(static_cast<TUnder>(a) & static_cast<TUnder>(b));    \
        return a;                                                                   \
    }                                                                               \
    inline TEnum &operator^=(TEnum &a, TEnum b)                                     \
    {                                                                               \
        using TUnder = typename std::underlying_type_t<TEnum>;                      \
        a = static_cast<TEnum>(static_cast<TUnder>(a) ^ static_cast<TUnder>(b));    \
        return a;                                                                   \
    }

namespace Lettuce::Core
{
    enum class BindPoint
    {
        Graphics = 0,
        Compute = 1,
        RayTracing = 1000165000,
        // Provided by VK_AMDX_shader_enqueue
        ExecutionGraph = 1000134000,
    };

    enum class AttachmentType
    {
        Color,
        DepthStencil,
        Input,
        //Preserve,
        Resolve,
    };

    enum class ImageLayout
    {
        Undefined = 0,
        General = 1,
        ColorAttachmentOptimal = 2,
        DepthStencilAttachmentOptimal = 3,
        DepthStencilReadOnlyOptimal = 4,
        ShaderReadOnlyOptimal = 5,
        TransferSrcOptimal = 6,
        TransferDstOptimal = 7,
        Preinitialized = 8,
        // Provided by VK_VERSION_1_1
        DepthReadOnlyStencilAttachmentOptimal = 1000117000,
        // Provided by VK_VERSION_1_1
        DepthAttachmentStencilReadOnlyOptimal = 1000117001,
        // Provided by VK_VERSION_1_2
        DepthAttachmentOptimal = 1000241000,
        // Provided by VK_VERSION_1_2
        DepthReadOnlyOptimal = 1000241001,
        // Provided by VK_VERSION_1_2
        StencilAttachmentOptimal = 1000241002,
        // Provided by VK_VERSION_1_2
        StencilReadOnlyOptimal = 1000241003,
        // Provided by VK_VERSION_1_3
        ReadOnlyOptimal = 1000314000,
        // Provided by VK_VERSION_1_3
        AttachmentOptimal = 1000314001,
        // Provided by VK_KHR_swapchain
        PresentSrc = 1000001002,
        // Provided by VK_KHR_video_decode_queue
        VideoDecodeDst = 1000024000,
        // Provided by VK_KHR_video_decode_queue
        VideoDecodeSrc = 1000024001,
        // Provided by VK_KHR_video_decode_queue
        VideoDecodeDpb = 1000024002,
        // Provided by VK_KHR_shared_presentable_image
        SharedPresent = 1000111000,
        // Provided by VK_EXT_fragment_density_map
        FragmentDensityMapOptimal = 1000218000,
        // Provided by VK_KHR_fragment_shading_rate
        FragmentShadingRateAttachmentOptimal = 1000164003,
        // Provided by VK_KHR_dynamic_rendering_local_read
        RenderingLocalRead = 1000232000,
        // Provided by VK_KHR_video_encode_queue
        VideoEncodeDst = 1000299000,
        // Provided by VK_KHR_video_encode_queue
        VideoEncodeSrc = 1000299001,
        // Provided by VK_KHR_video_encode_queue
        VideoEncodeDpb = 1000299002,
        // Provided by VK_EXT_attachment_feedback_loop_layout
        AttachmentFeedbackLoopOptimal = 1000339000,
    };

    enum class LoadOp
    {
        Load = 0,
        Clear = 1,
        DontCare = 2,
        // Provided by VK_KHR_load_store_op_none
        None = 1000400000,
    };

    enum class StoreOp
    {
        Store = 0,
        DontCare = 1,
        // Provided by VK_VERSION_1_3
        None = 1000301000,
    };

    enum class AccessBehavior
    {
        IndirectCommandRead = 0x00000001,
        IndexRead = 0x00000002,
        VertexAttributeRead = 0x00000004,
        UniformRead = 0x00000008,
        InputAttachmentRead = 0x00000010,
        ShaderRead = 0x00000020,
        ShaderWrite = 0x00000040,
        ColorAttachmentRead = 0x00000080,
        ColorAttachmentWrite = 0x00000100,
        DepthStencilAttachmentRead = 0x00000200,
        DepthStencilAttachmentWrite = 0x00000400,
        TransferRead = 0x00000800,
        TransferWrite = 0x00001000,
        HostRead = 0x00002000,
        HostWrite = 0x00004000,
        MemoryRead = 0x00008000,
        MemoryWrite = 0x00010000,
        // Provided by VK_VERSION_1_3
        None = 0,
        // Provided by VK_EXT_transform_feedback
        TransformFeedbackWrite = 0x02000000,
        // Provided by VK_EXT_transform_feedback
        TransformFeedbackCounterRead = 0x04000000,
        // Provided by VK_EXT_transform_feedback
        TransformFeedbackCounterWrite = 0x08000000,
        // Provided by VK_EXT_conditional_rendering
        ConditionalRenderingRead = 0x00100000,
        // Provided by VK_EXT_blend_operation_advanced
        ColorAttachmentReadNoncoherent = 0x00080000,
        // Provided by VK_KHR_acceleration_structure
        AccelerationStructureRead = 0x00200000,
        // Provided by VK_KHR_acceleration_structure
        AccelerationStructureWrite = 0x00400000,
        // Provided by VK_EXT_fragment_density_map
        FragmentDensityMapRead = 0x01000000,
        // Provided by VK_KHR_fragment_shading_rate
        FragmentShadingRateAttachmentRead = 0x00800000,
        // Provided by VK_NV_device_generated_commands
        CommandPreprocessRead = 0x00020000,
        // Provided by VK_NV_device_generated_commands
        CommandPreprocessWrite = 0x00040000,
    };
    MAKE_ENUM_FLAGS(AccessBehavior)

    enum class AccessStage
    {
        TopOfPipe = 0x00000001,
        DrawIndirect = 0x00000002,
        VertexInput = 0x00000004,
        VertexShader = 0x00000008,
        TessellationControlShader = 0x00000010,
        TessellationEvaluationShader = 0x00000020,
        GeometryShader = 0x00000040,
        FragmentShader = 0x00000080,
        EarlyFragmentTests = 0x00000100,
        LateFragmentTests = 0x00000200,
        ColorAttachmentOutput = 0x00000400,
        ComputeShader = 0x00000800,
        Transfer = 0x00001000,
        BottomOfPipe = 0x00002000,
        Host = 0x00004000,
        AllGraphics = 0x00008000,
        AllCommands = 0x00010000,
        None = 0,
        // Provided by VK_EXT_mesh_shader
        TaskShader = 0x00080000,
        // Provided by VK_EXT_mesh_shader
        MeshShader = 0x00100000,
    };

    enum class QueueType
    {
        Graphics,
        Present
    };

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

    void checkResult(const VkResult result, std::string onSucessMessage = "created successfully!");

    char *GetSurfaceExtensionNameByPlatform();

    template <typename T1, typename T2>
    VkResult CreateVkSurface(VkInstance instance, T1 window, T2 process, VkSurfaceKHR &surface, const VkAllocationCallbacks *allocator)
    {
#ifdef _WIN32
        VkWin32SurfaceCreateInfoKHR surfaceCI = {
            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .hinstance = (static_cast<HINSTANCE>(process)),
            .hwnd = (static_cast<HWND>(window)),
        };
        return vkCreateWin32SurfaceKHR(instance, &surfaceCI, allocator, &surface);
#else
        return VK_ERROR_INITIALIZATION_FAILED;
#endif
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);
}