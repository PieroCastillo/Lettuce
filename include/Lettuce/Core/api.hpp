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
#include "formats.hpp"

// external libs
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

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
    enum class Format : uint8_t;
    enum class Filter : uint8_t { Nearest, Linear };
    enum class SamplerAddressMode : uint8_t { Repeat, ClampToEdge, ClampToBorder, MirroredRepeat };
    enum class CompareOp : uint8_t { Never, Less, Equal, LessOrEqual, Greater, NotEqual, GreaterOrEqual, Always };
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
    enum class IndirectType : uint8_t { Draw, DrawIndexed, DrawMesh, Dispatch }; // TraceRays, DeviceGenerated

    // Resources
    struct BufferInfo {
        uint64_t size;
        void* cpuAddress;
        uint64_t gpuAddress;
        uint64_t alignment;
    };

    struct ResourceInfo
    {
        uint64_t size;
        uint32_t width;
        uint32_t height;
        uint64_t alignment;
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
        bool isCubeMap;
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
        Filter mipmap;
        SamplerAddressMode addressModeU;
        SamplerAddressMode addressModeV;
        SamplerAddressMode addressModeW;
        float anisotropy;
        bool depthCompare;
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
    };

    struct IndirectSetDesc {
        IndirectType type;
        uint32_t maxCount;
        uint32_t userDataSize;
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

        Buffer buffer;
        uint64_t offset;
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
        std::span<std::span<CommandBuffer>> commandBuffers;
        std::optional<Swapchain> presentSwapchain;
    };

    struct PushResourceDescriptorsDesc
    {
        std::span<const std::pair<uint32_t, Texture>> sampledTextures;
        std::span<const std::pair<uint32_t, Sampler>> samplers;
        std::span<const std::pair<uint32_t, Texture>> storageTextures;
        DescriptorTable  descriptorTable;
    };

    struct MemoryToMemoryCopy
    {
        MemoryView srcMemory;
        MemoryView dstMemory;
        uint64_t size;
    };

    struct MemoryToTextureCopy
    {
        MemoryView srcMemory;
        Texture dstTexture;
        uint32_t mipmapLevel;
        uint32_t layerBaseLevel;
        uint32_t layerCount;
    };
    
    struct PushAllocationsDesc 
    {
        std::span<const std::pair<uint32_t, MemoryView>> allocations;
        DescriptorTable descriptorTable;
    };

    struct ExecuteIndirectDesc
    {
        IndirectSet indirectSet;
        uint32_t maxDrawCount;
    };

    struct DeviceImpl;
    struct CommandBufferImpl { DeviceImpl* device; uint64_t handle; };

    struct Device {
    private:
        DeviceImpl* impl;
    public:
        void Create(const DeviceDesc&);
        void Destroy();

        void WaitFor(QueueType);

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

        void PushResourceDescriptors(const PushResourceDescriptorsDesc&);

        void PushAllocations(
            DescriptorTable,
            std::span<const std::pair<uint32_t, MemoryView>>
        );

        // Indirect Set
        IndirectSet CreateIndirectSet(const IndirectSetDesc&);
        void Destroy(IndirectSet);

        MemoryView GetIndirectSetView(IndirectSet);

        // Swapchain
        Swapchain CreateSwapchain(const SwapchainDesc&);
        void Destroy(Swapchain);

        void NextFrame(Swapchain);
        void DisplayFrame(Swapchain);
        Format GetRenderTargetFormat(Swapchain);
        RenderTarget GetCurrentRenderTarget(Swapchain) const;
        void ResizeSwapchain(Swapchain, uint32_t w, uint32_t h);
        uint32_t GetFrameCount(Swapchain);

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
        explicit CommandBuffer(CommandBufferImpl cmdImpl) : impl(cmdImpl) {}
    public:
        void MemoryCopy(const MemoryToMemoryCopy&);
        void MemoryCopy(const MemoryToTextureCopy&);

        void BeginRendering(const RenderPassDesc&);
        void EndRendering();

        void BindPipeline(Pipeline);
        void BindDescriptorTable(DescriptorTable, PipelineBindPoint);
        void PushAllocations(const PushAllocationsDesc&);

        void Draw(uint32_t vertexCount, uint32_t instanceCount);
        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount);
        void DrawMesh(uint32_t x, uint32_t y, uint32_t z);

        void ExecuteIndirect(const ExecuteIndirectDesc&);

        void Dispatch(uint32_t x, uint32_t y, uint32_t z);

        void Barrier(std::span<const BarrierDesc> barriers);
        void PrepareTexture(Texture);

        void Fill(MemoryView view, uint32_t value, uint32_t count);
    };
}
#endif // LETTUCE_CORE_API_HPP