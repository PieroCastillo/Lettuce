/*
Created by @PieroCastillo on 2026-05-17
*/
#ifndef LETTUCE_RENDERING_SCENE_TREE_HPP
#define LETTUCE_RENDERING_SCENE_TREE_HPP

#include "../Core/api.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Rendering
{
    struct GeometryClusterTemplateTag {};
    struct GeometryClusterInstanceTag {};
    struct ScenePartitionTag {};

    using GeometryClusterTemplate = Handle<GeometryClusterTemplateTag>;
    using GeometryClusterInstance = Handle<GeometryClusterInstanceTag>;
    using ScenePartition = Handle<ScenePartitionTag>;

    struct PrecomputedCluster
    {
        float3 bboxMin;
        float3 bboxMax;
        uint32_t indexOffset;
        uint32_t indexCount;
        uint32_t vertexOffset;
        uint32_t vertexCount;
    };

    enum class ClusterCompressionPrecision : uint8_t
    {
        Float32,
        Float16,
        Int16_Normalized,
        Int8_Normalized,
    };

    enum class SceneTreeFlags : uint32_t
    {
        None = 0,
        EnableQuantization = 1 << 0,
        GenerateConeCulling = 1 << 1,
        GenerateLODMetrics = 1 << 2,
    };

    constexpr SceneTreeFlags operator|(SceneTreeFlags a, SceneTreeFlags b) noexcept {
        return static_cast<SceneTreeFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    struct GeometryClusterTemplateDesc
    {
        uint32_t maxTotalClusters;
        uint32_t maxTotalVertices;
        ClusterCompressionPrecision precision;
        bool enableConeCompression;
    };

    struct GeometryClusterInstanceDesc {
        uint32_t maxClusterInstances;
    };

    struct ScenePartitionDesc {
        uint32_t maxInstanceCount;
    };

    struct BuildGeometryClusterTemplateDesc
    {
        GeometryClusterTemplate dstTemplate;
        SceneTreeFlags flags;
        MemoryView positions;
        uint64_t positionsOffset;
        uint32_t vertexStride;
        MemoryView indices;
        uint64_t indicesOffset;
        MemoryView precomputedClusters;
        uint64_t clustersOffset;
        uint32_t clusterCount;
    };

    struct BuildGeometryClusterInstanceDesc
    {
        GeometryClusterInstance dstInstance;
        GeometryClusterTemplate srcTemplate;
        MemoryView transforms;
        uint64_t transformsOffset;
        uint32_t instanceCount;
    };

    struct BuildScenePartitionDesc
    {
        ScenePartition dstPartition;
        MemoryView srcGeometryClusterInstances;
        uint64_t instancesOffset;
        uint32_t instanceCount;
    };

    struct UpdateGeometryClusterInstanceDesc
    {
        GeometryClusterInstance targetInstance;
        MemoryView dynamicTransforms;
        uint64_t transformsOffset;
        uint32_t instanceCount;
    };

    struct UpdateScenePartitionDesc
    {
        ScenePartition targetPartition;
        MemoryView updatedInstances;
        uint64_t instancesOffset;
        uint32_t instanceCount;
    };

    struct SceneTreeDesc
    {
        Device& device;
    };

    struct SceneTreeImpl;

    struct SceneTree
    {
    private:
        SceneTreeImpl* impl = nullptr;
    public:
        void Create(const SceneTreeDesc&);
        void Destroy();

        auto CreateTemplate(const GeometryClusterTemplateDesc&) -> GeometryClusterTemplate;
        auto CreateInstance(const GeometryClusterInstanceDesc&) -> GeometryClusterInstance;
        auto CreatePartition(const ScenePartitionDesc&) -> ScenePartition;

        auto GetPartitionAddress(ScenePartition) const -> DeviceAddress;
        auto GetTemplateAddress(GeometryClusterTemplate) const -> DeviceAddress;

        void Build(const BuildGeometryClusterTemplateDesc&);
        void Build(const BuildGeometryClusterInstanceDesc&);
        void Build(const BuildScenePartitionDesc&);

        void Update(const UpdateGeometryClusterInstanceDesc&);
        void Update(const UpdateScenePartitionDesc&);
    };

    struct SceneTreeCommandBuffer
    {
    private:
        const CommandBufferImpl* cmdImpl;
        SceneTree* sTree;
    public:
        explicit SceneTreeCommandBuffer(SceneTree& sceneTree, CommandBuffer& cmd) : sTree(&sceneTree), cmdImpl(&cmd.GetImplementation()) {}

        void Build(const BuildGeometryClusterTemplateDesc&);
        void Build(const BuildGeometryClusterInstanceDesc&);
        void Build(const BuildScenePartitionDesc&);

        void Update(const UpdateGeometryClusterInstanceDesc&);
        void Update(const UpdateScenePartitionDesc&);
    };
};

#endif