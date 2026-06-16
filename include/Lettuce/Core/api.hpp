/*
Created by @PieroCastillo on 2025-12-26
*/
#ifndef LETTUCE_CORE_API_HPP
#define LETTUCE_CORE_API_HPP

#include <array>
#include <cstdint>
#include <optional>
#include <ranges>
#include <span>
#include <string_view>
#include <variant>
#include "formats.hpp"

// external libs
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Lettuce::Core
{
    // types
    using float2 = glm::vec2;
    using float3 = glm::vec3;
    using float4 = glm::vec4;
    using float3x3 = glm::mat3;
    using float4x4 = glm::mat4;
    using DeviceAddress = uint64_t;
    using HostAddress = uint8_t*;

    template<typename Tag>
    struct Handle {
        uint32_t index = 0;
        uint32_t generation = 0;

        static constexpr Handle<Tag> Null() noexcept {
            return {};
        }

        constexpr bool valid() const noexcept {
            return generation != 0;
        }

        auto operator<=>(const Handle&) const = default;

        constexpr std::array<uint32_t, 2> get() const noexcept {
            return { index, generation };
        }
    };

    struct MemoryViewTag {};
    struct TextureViewTag {};
    struct SamplerTag {};
    struct ShaderBinaryTag {};
    struct PipelineTag {};
    struct DescriptorTableTag {};
    struct IndirectSetTag {};
    struct SwapchainTag {};
    struct CommandAllocatorTag {};

    using MemoryView = Handle<MemoryViewTag>;
    using TextureView = Handle<TextureViewTag>;
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
    enum class RenderTargetType : uint8_t { ColorRGB_sRGB, ColorRGBA_sRGB, Depth_D32 };
    enum class IndirectType : uint8_t { Draw, DrawIndexed, DrawMesh, Dispatch }; // TraceRays, DeviceGenerated

    // Resources
    struct MemoryViewInfo {
        uint64_t size;
        uint8_t* cpuAddress;
        uint64_t gpuAddress;
        uint64_t alignment;
    };

    struct TextureViewInfo
    {
        uint64_t size;
        uint32_t width;
        uint32_t height;
        uint64_t alignment;
    };

    // Descriptions
    struct ColorClear {
        std::array<float, 4> value;
    };

    struct DepthStencilClear {
        float depth;
        uint32_t stencil;
    };

    using ClearValue = std::variant<ColorClear, DepthStencilClear>;

    struct MemoryViewDesc {
        uint64_t size;
        bool cpuVisible;
    };

    struct TextureViewDesc {
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        Format format;
        uint32_t mipCount;
        uint32_t layerCount;
        bool isCubeMap;
        bool cpuVisible;
    };

    struct RenderTargetDesc {
        uint32_t width;
        uint32_t height;
        RenderTargetType type;
        ClearValue defaultClearValue;
        bool cpuVisible;
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

    struct AttachmentDesc
    {
        TextureView renderTarget;
        LoadOp loadOp;
    };

    struct RenderPassDesc
    {
        uint32_t width, height;
        std::span<const AttachmentDesc> colorAttachments;
        std::optional<const AttachmentDesc> depthStencilAttachment;
        std::optional<uint32_t> presentAttachmentIdx;
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
        std::span<const std::pair<uint32_t, TextureView>> sampledTextures;
        std::span<const std::pair<uint32_t, Sampler>> samplers;
        std::span<const std::pair<uint32_t, TextureView>> storageTextures;
        DescriptorTable  descriptorTable;
    };

    struct ClearTextureDesc
    {
        TextureView texture;
        ColorClear color;
        uint32_t baseLevel, levelCount, baseLayer, layerCount;
    };

    struct HostToMemoryCopy
    {
        std::span<const uint8_t> srcData;
        MemoryView dstMemory;
        uint32_t dstOffset;
    };

    struct HostToTextureCopy
    {
        std::span<const uint8_t> srcData;
        TextureView dstTexture;
        uint32_t mipmapLevel;
        uint32_t layerBaseLevel;
        uint32_t layerCount;
        uint32_t x, y, width, height;
    };

    struct MemoryToMemoryCopy
    {
        MemoryView srcMemory;
        MemoryView dstMemory;
        uint64_t size;
        uint32_t srcOffset;
        uint32_t dstOffset;
    };

    struct MemoryToTextureCopy
    {
        MemoryView srcMemory;
        TextureView dstTexture;
        uint32_t srcOffset;
        uint32_t mipmapLevel;
        uint32_t layerBaseLevel;
        uint32_t layerCount;
        uint32_t x, y, width, height;
    };

    struct TextureToMemory
    {
        TextureView srcTexture;
        MemoryView dstMemory;
        uint32_t mipmapLevel;
        uint32_t layerBaseLevel;
        uint32_t layerCount;
        uint32_t x, y, width, height;
        uint32_t dstOffset;
    };

    struct PushAllocationsDesc
    {
        std::span<const MemoryView> allocations;
        DescriptorTable descriptorTable;
    };

    struct ExecuteIndirectDesc
    {
        IndirectSet indirectSet;
        uint32_t offset;
        uint32_t maxDrawCount;
    };

    struct DeviceImpl;
    struct CommandBufferImpl { DeviceImpl* device; uint64_t handle; std::optional<TextureView> currentPresentTarget; };

    struct Device {
    private:
        DeviceImpl* impl;
    public:
        void Create(const DeviceDesc&);
        void Destroy();

        void WaitFor(QueueType);

        auto SupportMeshShader() -> bool;
        auto SupportNeuralShading() -> bool;
        auto SupportNeuralShadingNV() -> bool;
        auto SupportRayTracing() -> bool;
        auto SupportRayTracingNV() -> bool;
        auto SupportFragmentShadingRate() -> bool;
        auto QueryPreferredThreadCount() -> uint32_t;

        // MemoryView
        auto CreateMemoryView(const MemoryViewDesc&) -> MemoryView;
        void Destroy(MemoryView);

        // TextureView
        auto CreateTextureView(const TextureViewDesc&) -> TextureView;
        auto CreateTextureView(const RenderTargetDesc&) -> TextureView;
        void Destroy(TextureView);

        auto GetMemoryViewInfo(MemoryView) const -> MemoryViewInfo;
        auto GetResourceInfo(TextureView) const -> TextureViewInfo;

        // Sampler 
        auto CreateSampler(const SamplerDesc&) -> Sampler;
        void Destroy(Sampler);

        // Shader Binary 
        auto CreateShader(const ShaderBinaryDesc&) -> ShaderBinary;
        void Destroy(ShaderBinary);

        // Pipeline 
        auto CreatePipeline(const PrimitiveShadingPipelineDesc&) -> Pipeline;
        auto CreatePipeline(const MeshShadingPipelineDesc&) -> Pipeline;
        auto CreatePipeline(const ComputePipelineDesc&) -> Pipeline;
        void Destroy(Pipeline);

        // Descriptor Table
        auto CreateDescriptorTable(const DescriptorTableDesc&) -> DescriptorTable;
        void Destroy(DescriptorTable);

        void PushResourceDescriptors(const PushResourceDescriptorsDesc&);

        // Indirect Set
        auto CreateIndirectSet(const IndirectSetDesc&) -> IndirectSet;
        void Destroy(IndirectSet);

        auto GetIndirectSetView(IndirectSet) -> MemoryView;

        // Swapchain
        auto CreateSwapchain(const SwapchainDesc&) -> Swapchain;
        void Destroy(Swapchain);

        void NextFrame(Swapchain);
        void DisplayFrame(Swapchain);
        auto GetRenderTargetFormat(Swapchain) -> Format;
        auto GetCurrentRenderTarget(Swapchain) const -> TextureView;
        void ResizeSwapchain(Swapchain, uint32_t w, uint32_t h);
        auto GetFrameCount(Swapchain) -> uint32_t;

        // Command Allocator
        auto CreateCommandAllocator(const CommandAllocatorDesc&) -> CommandAllocator;
        void Destroy(CommandAllocator);

        void Reset(CommandAllocator);
        auto AllocateCommandBuffer(CommandAllocator) -> CommandBuffer;

        void Submit(const CommandBufferSubmitDesc&);

        void MemoryCopy(const HostToMemoryCopy&);
        void MemoryCopy(const HostToTextureCopy&);
        void MemoryCopy(const MemoryToMemoryCopy&);
        void MemoryCopy(const MemoryToTextureCopy&);

        [[nodiscard]] auto GetImplementation() noexcept -> DeviceImpl* { return impl; }
    };

    struct CommandBuffer
    {
    private:
        friend class Device;
        CommandBufferImpl impl;
        explicit CommandBuffer(CommandBufferImpl cmdImpl) : impl(cmdImpl) {}
    public:

        void MemoryCopy(const HostToMemoryCopy&);
        void MemoryCopy(const HostToTextureCopy&);
        void MemoryCopy(const MemoryToMemoryCopy&);
        void MemoryCopy(const MemoryToTextureCopy&);
        // Copy from offset[x,y], range[width, height] of the TextureView to the start of the buffer
        void MemoryCopy(const TextureToMemory&);
        void Fill(MemoryView view, uint32_t offset, uint32_t value, uint32_t count);

        void ClearTexture(const ClearTextureDesc&);

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
        void Barrier(std::initializer_list<BarrierDesc> barriers) { Barrier(std::span(barriers.begin(), barriers.end())); }

        void ResetCount(IndirectSet);

        [[nodiscard]] auto GetImplementation() noexcept -> CommandBufferImpl* { return &impl; }
    };
}
#endif // LETTUCE_CORE_API_HPP