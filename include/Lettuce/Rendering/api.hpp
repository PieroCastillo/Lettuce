/*
Created by @PieroCastillo on 2025-01-5
*/
#ifndef LETTUCE_RENDERING_API_HPP
#define LETTUCE_RENDERING_API_HPP

#include <any>
#include <array>
#include <atomic>
#include <cstdint>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <span>
#include <thread>
#include <typeindex>
#include <typeinfo>
#include <vector>

// project headers
#include "../Core/api.hpp"
#include "../Core/Allocators/IGPUMemoryResource.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Rendering
{
    using float2 = glm::vec2;
    using float3 = glm::vec3;
    using float4 = glm::vec4;
    using float3x3 = glm::mat3;
    using float4x4 = glm::mat4;

    struct IMaterial;
    struct SceneGraph;

    // BVH4
    struct BVHNodeGPUItem
    {
        float bboxMinX[4];
        float bboxMinY[4];
        float bboxMinZ[4];

        float bboxMaxX[4];
        float bboxMaxY[4];
        float bboxMaxZ[4];

        uint32_t childID[4]; // into a TLAS: node or instance, into a BLAS: node or meshlet
        uint32_t leafMask;
    };

    struct InstanceGPUItem
    {
        float3 aabbMin;
        float3 aabbMax;
        float4x4 model;
        float4x4 modelInv;
        uint32_t materialDataIdx;
        uint32_t meshletOffset;
        uint32_t meshletCount;
        uint32_t blasID;
    };

    // is 32 bytes enough?
    struct MaterialDataGPUItem
    {
        uint32_t materialID;
        uint32_t mmaterialData[7];
    };

    struct MeshletGPUItem
    {
        uint32_t vertexOffset;
        uint32_t triangleOffset;
        uint32_t vertexCount;
        uint32_t triangleCount;
        float3 center;
        float  radius;
    };

    struct VisibilityBufferGPUPixel
    {
        uint8_t LOD;
        uint8_t triangleID;
        uint16_t instanceID;
        uint32_t meshletID;
    };

    struct SceneGPUData
    {
        float4x4 view;
        float4x4 projection;
        float time;
        float timeDelta;
        uint32_t frameIndex;
        uint32_t lightsCount;
    };

    struct MaterialParams
    {
        std::vector<uint8_t> data;
        std::type_index type;
        uint32_t typeSize;

        template<typename T>
        void SetData(const T& value)
        {
            type = std::type_index(typeid(T));
            typeSize = sizeof(T);
            data.resize(sizeof(T));
            memcpy(data.data(), &value, sizeof(T));
        }

        template<typename T>
        T GetData() const
        {
            T value;
            memcpy(&value, data.data(), std::min(sizeof(T), data.size()));
            return value;
        }
    };

    struct IMaterial
    {
        Pipeline materialPipeline;
        DescriptorTable descriptorTable;
        std::type_index paramsType;
    };

    struct SceneGraphImpl;
    struct SceneNodeTag {};
    struct RenderableTag {};

    using SceneNode = Handle<SceneNodeTag>;
    using Renderable = Handle<RenderableTag>;

    struct SceneGraphDesc
    {
        Device& device;
        std::shared_ptr<Allocators::IGPUMemoryResource> allocator;
    };

    struct SceneGraph
    {
    private:
        SceneGraphImpl* impl;
    public:
        void Create(const SceneGraphDesc&);
        void Destroy();

        void AddLight();
        void ResetLights();

        auto CreateNode(const std::shared_ptr<IMaterial>&) -> SceneNode;
        void Destroy(SceneNode);

        void Record(CommandBuffer&);
    };

    struct AsyncRecorderDesc
    {
        Device& device;
        uint32_t threadCount;
    };

    struct AsyncRecorder
    {
    private:
        struct Task
        {
            uint32_t taskLevel;
            std::any userData;
            std::move_only_function<void(CommandBuffer, std::any)> recordFunc;
        };

        Device* device;
        std::vector<std::jthread> threads;
        std::vector<CommandAllocator> allocators;

        // synchronization objects
        std::atomic<bool> stop;
        std::mutex tasksMutex;
        std::queue<Task> taskQueue;
        std::condition_variable taskCV;

        std::mutex cmdsMutex;
        std::vector<std::vector<CommandBuffer>> cmdLevels;

        std::atomic<uint32_t> pendingTasks;
        std::atomic<uint32_t> currentLevel;
    public:
        void Create(const AsyncRecorderDesc& desc);
        void Destroy();

        void Reset();

        void RecordAsync(std::any, std::move_only_function<void(CommandBuffer, std::any)>&&);
        void Barrier();

        void Submit(std::optional<Swapchain> optSwapchain = std::nullopt);
    };
};
#endif // LETTUCE_RENDERING_API_HPP