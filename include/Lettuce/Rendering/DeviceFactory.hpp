/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_DEVICE_HPP
#define LETTUCE_CORE_DEVICE_HPP

// standard headers
#include <concepts>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

// project headers
#include "Allocators/GPUMonotonicBufferResource.hpp"
#include "Allocators/LinearImageAlloc.hpp"
#include "Allocators/LinearBufferSubAlloc.hpp"
#include "common.hpp"
#include "DescriptorTable.hpp"
#include "DeviceExecutionContext.hpp"
#include "DeviceVector.hpp"
#include "Pipeline.hpp"
#include "RenderFlowGraph.hpp"
#include "Swapchain.hpp"
#include "Sampler.hpp"
#include "SequentialExecutionContext.hpp"
#include "ShaderPack.hpp"
#include "TextureDictionary.hpp"

namespace Lettuce::Core
{
    using shader = std::tuple<std::string, std::weak_ptr<ShaderPack>>;

    struct GraphicsPipelineCreateData
    {
        std::weak_ptr<ShaderPack> shaders;
        std::weak_ptr<DescriptorTable> descriptorTable;
        std::vector<std::weak_ptr<RenderTarget>> colorTargets;
        std::weak_ptr<RenderTarget> depthTarget;
        // "classic" rasterization pipeline
        std::vector<VertexInput> inputs;
        std::optional<shader> vertexShader;
        // mesh rasterization pipeline
        std::optional<shader> taskShader;
        std::optional<shader> meshShader;
        shader fragmentShader;
    };

    // for the future
    struct RayTracingPipelineCreateData
    {
        std::weak_ptr<ShaderPack> shaders;
        std::weak_ptr<DescriptorTable> descriptorTable;
        std::optional<std::string> rayGenEntryPoint;
        std::optional<std::string> anyHitEntryPoint;
        std::optional<std::string> closestHitEntryPoint;
        std::optional<std::string> missEntryPoint;
        std::optional<std::string> intersectionEntryPoint;
        std::optional<std::string> callableEntryPoint;
    };

    struct ComputePipelineCreateData
    {
        std::weak_ptr<ShaderPack> shaders;
        std::weak_ptr<DescriptorTable> descriptorTable;
        std::string computeEntryPoint;
    };

    struct TextureCreateData
    {
        std::vector<std::pair<std::string, std::string>> namePathPairs;
    };

    struct Features
    {
        bool FragmentShadingRate;
        bool ExecutionGraphs;
        bool MeshShading;
        bool RayTracing;
        bool Video;
    };

    struct EnabledRecommendedFeatures
    {
        bool shaderObject = false;
        bool deviceGeneratedCommands = false;
        bool graphicsPipelineLibrary = false;
        bool dynamicRenderingLocalRead = false;
        bool maintenance5 = false;
    };

    enum class DeviceQueueType
    {
        Graphics,
        Compute,
        Transfer,
    };

    struct QueueFamilyIndices
    {
        uint32_t graphics;
        uint32_t compute;
        uint32_t transfer;
    };

    struct DeviceCreateInfo
    {
        bool preferDedicated;
    };

    class DeviceFactory
    {
        // copy objects
        VkCommandBuffer m_copyCmd;
        VkCommandPool m_copyCmdPool;
        VkFence m_copyFence;
        std::vector<std::tuple<VkBuffer, VkBuffer, VkBufferCopy>> m_copies; // src, dst, copy


        template <typename T>
        using Result = std::expected<std::shared_ptr<T>, LettuceResult>;
        using Op = std::expected<void, LettuceResult>;

        auto CreateContext(const DeviceExecutionContextCreateInfo& createInfo) -> Result<DeviceExecutionContext>;
        auto CreateDescriptorTable(const DescriptorTableCreateInfo& createInfo) -> Result<DescriptorTable>;

        template<typename T>
        auto CreateDeviceVector(const DeviceVectorCreateInfo& createInfo) -> Result<DeviceVector<T>>;

        template<ICommandRecordingContext... Contexts>
        auto CreateGraph() -> Result<RenderFlowGraph<Contexts...>>;
        auto CreatePipeline(const ComputePipelineCreateData& data) -> Result<Pipeline>;
        auto CreatePipeline(const GraphicsPipelineCreateData& data) -> Result<Pipeline>;
        auto CreateRenderTarget(const RenderTargetCreateInfo& createInfo) -> Result<RenderTarget>;
        // auto CreateSampler() -> Result<Sampler>;
        auto CreateSequentialContext() -> Result<SequentialExecutionContext>;
        auto CreateShaderPack(const ShaderPackCreateInfo& createInfo) -> Result<ShaderPack>;
        auto CreateSwapchain(const SwapchainCreateInfo& createInfo) -> Result<Swapchain>;
        // auto CreateTableGroup() -> Result<TableGroup>;
        auto CreateTextureDictionary(const TextureCreateData& createData) -> Result<TextureDictionary>;

        // allocators:
        auto CreateGPUMonotonicBufferResource(const Allocators::GPUMonotonicBufferResourceCreateInfo& createInfo) -> Result<Allocators::GPUMonotonicBufferResource>;
        auto CreateLinearImageAllocator(const Allocators::LinearImageAllocCreateInfo& createInfo) -> Result<Allocators::LinearImageAlloc>;
        auto CreateLinearBufferSuballocator(const Allocators::LinearBufferSubAllocCreateInfo& createInfo) -> Result<Allocators::LinearBufferSubAlloc>;

        auto MemoryCopy(const DeviceVectorBase& src, const DeviceVectorBase& dst, uint32_t srcIdx, uint32_t dstIdx, uint32_t count) -> void;
        auto FlushCopies() -> void;

        template<typename T>
        auto MemoryCopy(const std::shared_ptr<DeviceVector<T>>& src, const std::shared_ptr<DeviceVector<T>>& dst, uint32_t count, uint32_t srcIdx = 0, uint32_t dstIdx = 0) -> void;


    };
}
#include "Device.inl"
#endif // LETTUCE_CORE_DEVICE_HPP