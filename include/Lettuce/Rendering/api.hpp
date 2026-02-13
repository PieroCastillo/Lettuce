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
    struct IMaterial;
    struct SceneGraph;

    struct BVHNodeGPUItem
    {
        glm::vec3 aabbMin;
        uint32_t  leftFirst;
        glm::vec3 aabbMax;
        uint32_t  count;
    };

    struct TLASGPUItem
    {
        uint32_t rootNodeIdx;
    };

    struct BLASGPUItem
    {
        uint32_t rootNodeIdx;
    };

    struct InstanceGPUItem
    {
        glm::vec3 aabbMin;
        uint32_t meshletIdx;
        glm::vec3 aabbMax;
        uint32_t materialDataIdx;
        glm::mat4 model;
        glm::mat4 modelInv;
    };

    // 16 bytes is enough
    struct MaterialDataGPUItem
    {
        uint32_t materialID;
        uint8_t  mmaterialData[15];
    };

    struct MeshletGPUItem
    {
        uint32_t vertexOffset;
        uint32_t triangleOffset;
        uint32_t vertexCount;
        uint32_t triangleCount;
        glm::vec3 center;
        float  radius;
    };

    struct VisibilityBufferGPUPixel
    {
        uint32_t instanceIdxLOD; // instanceIdx: 24 bits, LOD: 8 bits
        uint32_t meshletIdx;
    };

    struct LightGPUData
    {
        glm::vec3 color;
        glm::vec3 direction;
        float intensity;
    };

    struct SceneGPUData
    {
        glm::mat4 view;
        glm::mat4 projection;
        float time;
        float timeDelta;
        uint32_t frameIndex;
        uint32_t lightsCount;
    };

    // plane: vec3 to vec3                          (vec3 + vec3)
    // box: vec3 to vec3 or center + side           (vec3 + float)
    // sphere: center + radious + detail1 + detail2 (vec3 + float + uint2)
    struct GeometryGPUData
    {
        glm::mat4 model;
        // procedural gen data
        glm::vec3 data0;
        glm::vec3 data1;
        // mesh buffer offsets
        uint32_t indexOffset;
        uint32_t vertexOffset;
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