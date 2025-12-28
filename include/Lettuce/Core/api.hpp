/*
Created by @PieroCastillo on 2025-12-26
*/
#ifndef LETTUCE_CORE_API_HPP
#define LETTUCE_CORE_API_HPP

#include <span>
#include <cstdint>
#include <string_view>

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

    struct AllocationTag {};
    struct TextureTag {};
    struct RenderTargetTag {};
    struct SamplerTag {};
    struct ShaderBinaryTag {};
    struct PipelineTag {};
    struct DescriptorTableTag {};
    struct IndirectSetTag {};
    struct SwapchainTag {};

    using Allocation = Handle<AllocationTag>;
    using Texture = Handle<TextureTag>;
    using RenderTarget = Handle<RenderTargetTag>;
    using Sampler = Handle<SamplerTag>;
    using ShaderBinary = Handle<ShaderBinaryTag>;
    using Pipeline = Handle<PipelineTag>;
    using DescriptorTable = Handle<DescriptorTableTag>;
    using IndirectSet = Handle<IndirectSetTag>;
    using Swapchain = Handle<SwapchainTag>;

    // Enums
    enum class Format : uint8_t {};
    enum class Filter : uint8_t {};
    enum class SamplerAddressMode : uint8_t {};
    enum class CompareOp : uint8_t {};
    enum class BorderColor : uint8_t {};
    enum class PrimitiveTopology : uint8_t {};

    enum class RenderTargetType : uint8_t {};
    enum class IndirectType : uint8_t { Draw, DrawIndexed, DrawMesh, Dispatch, DeviceGenerated };

    // Resources
    struct TextureInfo {
        uint64_t size;
        Format format;
    };

    struct AllocationInfo {
        uint64_t size;
        void* cpuAddress;
        uint64_t gpuAddress;
    };

    struct RenderTargetInfo
    {
        uint32_t width;
        uint32_t height;
        RenderTargetType type;
    };

    // Descriptions
    struct AllocationDesc {
        uint64_t size;
        bool cpuVisible;
    };

    struct TextureDesc {
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        Format format;
        uint32_t mipLevels;
    };

    struct RenderTargetDesc {
        uint32_t width;
        uint32_t height;
        RenderTargetType type;
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
        std::string_view taskEntryPoint;
        std::string_view meshEntryPoint;
        std::string_view fragEntryPoint;
        ShaderBinary  taskShaderBinary;
        ShaderBinary  meshShaderBinary;
        ShaderBinary  fragShaderBinary;
        std::span<Format> colorAttachmentFormats;
        Format depthAttachmentFormat;
        Format stencilAttachmentFormat;
        DescriptorTable  descriptorTable;
    };

    struct PrimitiveShadingPipelineDesc
    {
        PrimitiveTopology topology;
        bool fragmentShadingRate;
        std::string_view vertEntryPoint;
        std::string_view fragEntryPoint;
        ShaderBinary  vertShaderBinary;
        ShaderBinary  fragShaderBinary;
        std::span<Format> colorAttachmentFormats;
        Format depthAttachmentFormat;
        Format stencilAttachmentFormat;
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
        Allocation backingBuffer;
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

    struct DeviceImpl;
    struct Device {
    private:
        DeviceImpl* impl;
    public:
        // Memory 
        Allocation CreateAllocation(const AllocationDesc&);
        void Destroy(Allocation);

        AllocationInfo GetAllocationInfo(Allocation) const;

        // Textures 
        Texture  CreateTexture(const TextureDesc&);
        void Destroy(Texture);

        TextureInfo GetTextureInfo(Texture) const;

        // Render Targets
        RenderTarget CreateRenderTarget(const RenderTargetDesc&);
        void Destroy(RenderTarget);

        RenderTargetInfo GetRenderTargetInfo(RenderTarget) const;

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

        //  Descriptor Table
        DescriptorTable  CreateDescriptorTable(const DescriptorTableDesc&);
        void Destroy(DescriptorTable);

        void PushDescriptors(
            DescriptorTable,
            std::span<const std::pair<uint32_t, Texture>>,
            std::span<const std::pair<uint32_t, Sampler>>,
            std::span<const std::pair<uint32_t, Texture>>);

        // Indirect Set
        IndirectSet CreateIndirectSet(const IndirectSetDesc&);
        void Destroy(IndirectSet);

        // Swapchain
        Swapchain CreateSwapchain(const SwapchainDesc&);
        void Destroy(Swapchain);

        void NextFrame(Swapchain);
        void DisplayFrame(Swapchain);
        RenderTarget GetCurrentRenderTarget(Swapchain) const;
        void ResizeSwapchain(Swapchain, uint32_t w, uint32_t h);
    };

    struct CommandBuffer
    {
        void MemoryCopy(
            Allocation src,
            Allocation dst,
            uint64_t srcOffset,
            uint64_t dstOffset,
            uint64_t size
        );

        void BeginRendering(std::span<RenderTarget>);
        void EndRendering();

        void BindPipeline(Pipeline);
        void BindDescriptorTable(DescriptorTable);

        void Draw(uint32_t vertexCount, uint32_t instanceCount);
        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount);
        void DrawMesh(uint32_t x, uint32_t y, uint32_t z);

        void ExecuteIndirect(IndirectSet);

        void Dispatch(uint32_t x, uint32_t y, uint32_t z);
    };

    struct CommandPool {
        CommandBuffer Allocate();
        void Reset();
    };
}
#endif // LETTUCE_CORE_API_HPP