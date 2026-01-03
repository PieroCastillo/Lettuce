/*
Created by @PieroCastillo on 2025-12-26
*/
#ifndef LETTUCE_CORE_API_HPP
#define LETTUCE_CORE_API_HPP

#include <array>
#include <cstdint>
#include <span>
#include <string_view>
#include <optional>
#include <variant>

namespace Lettuce::Core
{
    template<typename Tag>
    struct Handle {
        uint32_t index = 0;
        uint32_t generation = 0;

        constexpr bool valid() const noexcept {
            return generation != 0;
        }

        auto operator<=>(const Handle&) const = default;
    };

    struct MemoryHeapTag {};
    struct BufferTag {};
    struct TextureTag {};
    struct RenderTargetTag {};
    struct SamplerTag {};
    struct ShaderBinaryTag {};
    struct PipelineTag {};
    struct DescriptorTableTag {};
    struct IndirectSetTag {};
    struct SwapchainTag {};
    struct CommandAllocatorTag {};

    using MemoryHeap = Handle<MemoryHeapTag>;
    using Buffer = Handle<BufferTag>;
    using Texture = Handle<TextureTag>;
    using RenderTarget = Handle<RenderTargetTag>;
    using Sampler = Handle<SamplerTag>;
    using ShaderBinary = Handle<ShaderBinaryTag>;
    using Pipeline = Handle<PipelineTag>;
    using DescriptorTable = Handle<DescriptorTableTag>;
    using IndirectSet = Handle<IndirectSetTag>;
    using Swapchain = Handle<SwapchainTag>;
    using CommandAllocator = Handle<CommandAllocatorTag>;

    // Enums
    enum class Format : uint8_t {
        RGBA8_UNORM = 0,
        RGBA8_SRGB,
        BGRA8_UNORM,
        BGRA8_SRGB,
        RGBA16_FLOAT,
        D16,
        D32,
        D24S8,
        D32S8,
        R8,
        RG8,
        RG16_FLOAT,
        COUNT
    };
    enum class Filter : uint8_t {};
    enum class SamplerAddressMode : uint8_t {};
    enum class CompareOp : uint8_t {};
    enum class BorderColor : uint8_t {};
    enum class PrimitiveTopology : uint8_t {};
    enum class PipelineBindPoint : uint8_t { Graphics, Compute, RayTracing, Count };
    enum class PipelineAccess : uint8_t { Read, Write };
    enum class PipelineStage : uint8_t {
        None,
        DrawIndirect,
        TaskShader, MeshShader, VertexShader, FragmentShader,
        EarlyFragmentTests, LateFragmentTests,
        ColorAttachmentOutput,
        ComputeShader,
        Copy, Blit, Resolve, Clear,
        RayTracingShader,
        CommandPreprocess,
        Count,
    };
    enum class LoadOp : uint8_t { Load, Clear, None, Count };

    enum class QueueType : uint8_t { Graphics, Compute, Copy };
    enum class RenderTargetType : uint8_t { ColorRGB_sRGB, ColorRGBA_sRGB, DepthStencilDS40 };
    enum class IndirectType : uint8_t { Draw, DrawIndexed, DrawMesh, Dispatch, DeviceGenerated };

    // Resources
    struct BufferInfo {
        uint64_t size;
        void* cpuAddress;
        uint64_t gpuAddress;
    };

    struct ResourceInfo
    {
        uint64_t size;
        uint32_t width;
        uint32_t height;
    };

    // Descriptions
    struct MemoryHeapDesc {
        uint64_t size;
        bool cpuVisible;
    };

    struct MemoryBindDesc {
        MemoryHeap heap;
        uint64_t heapOffset;
    };

    struct BufferDesc {
        uint64_t size;
    };

    struct TextureDesc {
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        Format format;
        uint32_t mipCount;
        uint32_t layerCount;
    };

    struct ColorClear {
        std::array<float, 4> value;
    };

    struct DepthStencilClear {
        float depth;
        uint32_t stencil;
    };

    using ClearValue = std::variant<ColorClear, DepthStencilClear>;

    struct RenderTargetDesc {
        uint32_t width;
        uint32_t height;
        RenderTargetType type;
        ClearValue defaultClearValue;
    };

    struct SamplerDesc {
        Filter magFilter;
        Filter minFilter;
        SamplerAddressMode addressModeU;
        SamplerAddressMode addressModeV;
        SamplerAddressMode addressModeW;
        float mipLodBias;
        bool anisotropyEnable;
        float maxAnisotropy;
        bool compareEnable;
        CompareOp compareOp;
        float minLod;
        float maxLod;
        BorderColor borderColor;
    };

    struct ShaderBinaryDesc {
        std::span<uint32_t> bytecode;
    };

    struct MeshShadingPipelineDesc
    {
        bool fragmentShadingRate;
        std::optional<std::string_view> taskEntryPoint;
        std::string_view meshEntryPoint;
        std::string_view fragEntryPoint;
        std::optional<ShaderBinary>  taskShaderBinary;
        ShaderBinary  meshShaderBinary;
        ShaderBinary  fragShaderBinary;
        std::span<Format> colorAttachmentFormats;
        std::optional<Format> depthStencilAttachmentFormat;
        DescriptorTable  descriptorTable;
    };

    struct PrimitiveShadingPipelineDesc
    {
        bool fragmentShadingRate;
        std::string_view vertEntryPoint;
        std::string_view fragEntryPoint;
        ShaderBinary  vertShaderBinary;
        ShaderBinary  fragShaderBinary;
        std::span<Format> colorAttachmentFormats;
        std::optional<Format> depthStencilAttachmentFormat;
        DescriptorTable  descriptorTable;
    };

    struct ComputePipelineDesc
    {
        std::string_view compEntryPoint;
        ShaderBinary  compShaderBinary;
        DescriptorTable  descriptorTable;
    };

    struct DescriptorTableDesc
    {
        uint32_t sampledImageDescriptorCount;
        uint32_t samplerDescriptorCount;
        uint32_t storageImageDescriptorCount;
        uint32_t bufferPointerCount;
    };

    struct IndirectSetDesc {
        IndirectType type;
        uint32_t maxCount;
        uint32_t stride;
    };

    struct SwapchainDesc
    {
        uint32_t width;
        uint32_t height;
        bool clipped;
        void* windowPtr;
        void* applicationPtr;
    };

    struct BarrierDesc {
        PipelineAccess srcAccess;
        PipelineStage srcStage;
        PipelineAccess dstAccess;
        PipelineStage dstStage;
    };

    struct MemoryView
    {
        uint64_t size;
        void* cpuAddress;
        uint64_t gpuAddress;
    };

    struct AttachmentDesc
    {
        RenderTarget renderTarget;
        LoadOp loadOp;
    };

    struct RenderPassDesc
    {
        uint32_t width, height;
        std::span<const AttachmentDesc> colorAttachments;
        std::optional<const AttachmentDesc> depthStencilAttachment;
    };

    struct DeviceDesc
    {
        bool preferDedicated;
    };

    struct CommandAllocatorDesc
    {
        QueueType queueType;
    };

    struct Device;
    struct CommandBuffer;

    struct CommandBufferSubmitDesc
    {
        QueueType queueType;
        std::span<const CommandBuffer> commandBuffers;
    };

    struct DeviceImpl;
    struct CommandBufferImpl { DeviceImpl* device; uint64_t handle; };

    struct Device {
    private:
        DeviceImpl* impl;
    public:
        void Create(const DeviceDesc&);
        void Destroy();

        // Memory 
        MemoryHeap CreateMemoryHeap(const MemoryHeapDesc&);
        void Destroy(MemoryHeap);

        // Buffer
        Buffer CreateBuffer(const BufferDesc&, const MemoryBindDesc&);
        void Destroy(Buffer);

        // Texture
        Texture CreateTexture(const TextureDesc&, const MemoryBindDesc&);
        void Destroy(Texture);

        // Render Target
        RenderTarget CreateRenderTarget(const RenderTargetDesc&, const MemoryBindDesc&);
        void Destroy(RenderTarget);

        BufferInfo GetBufferInfo(Buffer) const;
        ResourceInfo GetResourceInfo(Texture) const;
        ResourceInfo GetResourceInfo(RenderTarget) const;

        // Sampler 
        Sampler CreateSampler(const SamplerDesc&);
        void Destroy(Sampler);

        // Shader Binary 
        ShaderBinary CreateShader(const ShaderBinaryDesc&);
        void Destroy(ShaderBinary);

        // Pipeline 
        Pipeline CreatePipeline(const PrimitiveShadingPipelineDesc&);
        Pipeline CreatePipeline(const MeshShadingPipelineDesc&);
        Pipeline CreatePipeline(const ComputePipelineDesc&);
        void Destroy(Pipeline);

        // Descriptor Table
        DescriptorTable CreateDescriptorTable(const DescriptorTableDesc&);
        void Destroy(DescriptorTable);

        void PushResourceDescriptors(
            DescriptorTable,
            std::span<const std::pair<uint32_t, Texture>>,
            std::span<const std::pair<uint32_t, Sampler>>,
            std::span<const std::pair<uint32_t, Texture>>);

        void PushAllocations(
            DescriptorTable,
            std::span<const std::pair<uint32_t, const MemoryView&>>
        );

        // Indirect Set
        IndirectSet CreateIndirectSet(const IndirectSetDesc&);
        void Destroy(IndirectSet);

        // Swapchain
        Swapchain CreateSwapchain(const SwapchainDesc&);
        void Destroy(Swapchain);

        void NextFrame(Swapchain);
        void DisplayFrame(Swapchain);
        Format GetRenderTargetFormat(Swapchain);
        RenderTarget GetCurrentRenderTarget(Swapchain) const;
        void ResizeSwapchain(Swapchain, uint32_t w, uint32_t h);

        // Command Allocator
        CommandAllocator CreateCommandAllocator(const CommandAllocatorDesc&);
        void Destroy(CommandAllocator);

        void Reset(CommandAllocator);
        CommandBuffer AllocateCommandBuffer(CommandAllocator);

        void Submit(const CommandBufferSubmitDesc&);
    };

    struct CommandBuffer
    {
    private:
        friend class Device;
        CommandBufferImpl impl;
        explicit CommandBuffer(CommandBufferImpl cmdImpl): impl(cmdImpl) {}
    public:
        void MemoryCopy(
            const MemoryView& src,
            const MemoryView& dst,
            uint64_t srcOffset,
            uint64_t dstOffset,
            uint64_t size
        );

        void BeginRendering(const RenderPassDesc&);
        void EndRendering();

        void BindPipeline(Pipeline);
        void BindDescriptorTable(DescriptorTable, PipelineBindPoint);

        void Draw(uint32_t vertexCount, uint32_t instanceCount);
        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount);
        void DrawMesh(uint32_t x, uint32_t y, uint32_t z);

        void ExecuteIndirect(IndirectSet);

        void Dispatch(uint32_t x, uint32_t y, uint32_t z);

        void Barrier(std::span<const BarrierDesc> barriers);
    };
}
#endif // LETTUCE_CORE_API_HPP