implementing LettuceRendering;

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
    uint64_t geometryClusterAddr;
    float4x4 transform;
    float explicitAABB[6];
    uint32_t instanceID;
    uint32_t instanceMask;
    uint32_t instanceContributionToHitGroupIndex;
    uint32_t flags;
    uint32_t instanceIndex;
    uint32_t partitionIndex;
};

struct ScenePartitionUpdateIndirectCommand
{
    uint64_t geometryClusterAddr;
    uint32_t instanceIndex;
    uint32_t instanceContributionToHitGroupIndex;
};