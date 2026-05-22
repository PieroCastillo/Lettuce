/*
Created by @PieroCastillo on 2026-05-17
*/
#ifndef LETTUCE_RENDERING_SCENE_TREE_HPP
#define LETTUCE_RENDERING_SCENE_TREE_HPP

#include "../Core/api.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Rendering
{
    struct GeometryClusterTag {};
    struct ScenePartitionTag {};

    using GeometryCluster = Handle<GeometryClusterTag>;
    using ScenePartition = Handle<ScenePartitionTag>;

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

    enum class ScenePartitionFlags : uint32_t
    {
        None = 0,
        EnablePartitionTranslation = 1 << 0,
    };

    enum class ScenePartitionInstanceFlags : uint32_t
    {
        None = 0,
        ForceVisible = 1 << 0,
        DisableOcclusionCulling = 1 << 1,
        DisableFrustumCulling = 1 << 2,
        Static = 1 << 3,
    };

    constexpr SceneTreeFlags operator|(SceneTreeFlags a, SceneTreeFlags b) noexcept
    {
        return static_cast<SceneTreeFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    constexpr ScenePartitionFlags operator|(ScenePartitionFlags a, ScenePartitionFlags b) noexcept
    {
        return static_cast<ScenePartitionFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    constexpr ScenePartitionInstanceFlags operator|(ScenePartitionInstanceFlags a, ScenePartitionInstanceFlags b) noexcept
    {
        return static_cast<ScenePartitionInstanceFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    struct ClusterBuildIndirectCommand
    {
        float3 bboxMin;
        float3 bboxMax;
        float4 cone;
        float lodError;
        uint32_t clusterID;
        uint32_t geometryIndex;
        uint32_t indexOffset;
        uint32_t indexCount;
        uint32_t vertexOffset;
        uint32_t vertexCount;
    };
    
    struct ScenePartitionBuildIndirectCommand
    {
        DeviceAddress geometryClusterAddr;
        float4x4 transform;
        float explicitAABB[6];
        uint32_t instanceID;
        uint32_t instanceMask;
        uint32_t instanceContributionToHitGroupIndex;
        ScenePartitionInstanceFlags flags;
        uint32_t instanceIndex;
        uint32_t partitionIndex;
    };

    struct ScenePartitionUpdateIndirectCommand
    {
        DeviceAddress geometryClusterAddr;
        uint32_t instanceIndex;
        uint32_t instanceContributionToHitGroupIndex;
    };

    struct GeometryClusterDesc
    {
        uint32_t maxClusterCount;
        uint32_t maxVertexCount;
        ClusterCompressionPrecision precision;
        bool enableConeCompression;
    };

    struct ScenePartitionDesc
    {
        uint32_t maxInstanceCount;
        uint32_t maxPartitionCount;
        uint32_t maxInstancePerPartitionCount;
        uint32_t maxGlobalPartitionInstanceCount;
        ScenePartitionFlags flags;
    };

    struct BuildGeometryClusterDesc
    {
        GeometryCluster dstCluster;
        SceneTreeFlags flags;
        MemoryView positions;
        uint64_t positionsByteOffset;
        uint32_t vertexStride;
        MemoryView indices;
        uint64_t indicesByteOffset;
        MemoryView clusterDescs;
        uint64_t clusterDescsByteOffset;
        uint32_t clusterCount;
    };

    struct BuildScenePartitionDesc
    {
        ScenePartition dstPartition;
        MemoryView instanceWrites;
        uint64_t instanceWritesByteOffset;
        uint32_t instanceCount;
    };

    struct UpdateScenePartitionDesc
    {
        ScenePartition dstPartition;
        MemoryView instanceUpdates;
        uint64_t instanceUpdatesByteOffset;
        uint32_t instanceUpdateCount;
    };

    struct SceneTreeDesc
    {
        Device& device;
        uint32_t maxPartitionCount;
        uint32_t maxTotalInstancedGeometryClusterCount;
        uint32_t maxTotalClusterCount;
    };

    struct SceneTreeImpl;
    struct SceneTreeCommandBuffer;

    struct SceneTree
    {
    private:
        friend class SceneTreeCommandBuffer;
        SceneTreeImpl* impl = nullptr;
    public:
        void Create(const SceneTreeDesc&);
        void Destroy();

        auto CreateCluster(const GeometryClusterDesc&) -> GeometryCluster;
        auto CreatePartition(const ScenePartitionDesc&) -> ScenePartition;

        auto GetClusterAddress(GeometryCluster) const -> DeviceAddress;
        auto GetPartitionAddress(ScenePartition) const -> DeviceAddress;
    };

    struct SceneTreeCommandBuffer
    {
    private:
        uint64_t cmdHandle;
        SceneTree* sTree;
    public:
        explicit SceneTreeCommandBuffer(SceneTree& sceneTree, CommandBuffer& cmd) : sTree(&sceneTree), cmdHandle(cmd.GetImplementation()->handle) {}

        void Build(const BuildGeometryClusterDesc&);
        void Build(const BuildScenePartitionDesc&);
        void Update(const UpdateScenePartitionDesc&);
    };
};
#endif // LETTUCE_RENDERING_SCENE_TREE_HPP