implementing LettuceRendering;

struct BuildGeometryClusterPayload
{
    uint64_t dstGeometryClusterAddr;
    uint64_t clustersAddr;
    uint64_t positionsAddr;
    uint64_t indicesAddr;
    uint64_t scratchAddr;

    uint32_t clusterCount;
    uint32_t positionStride;
    uint32_t flags;
};

struct BuildScenePartitionPayload
{
    uint64_t dstScenePartitionAddr;
    uint64_t instancesAddr;
    uint64_t instanceBuildsAddr;
    uint64_t scratchAddr;
    uint32_t instanceCount;
    uint32_t instanceBuildCount;
};

struct UpdateScenePartitionPayload
{
    uint64_t dstPartitionAddr;
    uint64_t instancesAddr;
    uint64_t instanceUpdatesAddr;
    uint64_t scratchAddr;
    uint32_t instanceCount;
    uint32_t instanceUpdateCount;
};

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

struct BVH2NodeStorage
{
    float3 aabbMin;
    uint32_t leftChild;
    float3 aabbMax;
    uint32_t rightChild;
    uint32_t parent;
    uint32_t primitiveCount;
    uint32_t firstPrimitive;
    uint32_t flags;
};

struct BVH8NodeStorage
{
    float3 aabbMin[8];
    float3 aabbMax[8];

    uint32_t parentNodeIdx[8];
    uint32_t childBaseIdx[8];
    uint32_t childCount[8];
    uint32_t validChildMask[8];
};

struct ScenePartitionStorage
{
    uint64_t nodesAddr;
    uint64_t nodeCount;
    uint64_t bvh8RootAddr;
};

struct ScenePartitionNodeStorage
{
    uint64_t geometryClusterAddr;
    float4x4 transform;
    float4 cone;
};

struct GeometryClusterStorage
{
    uint64_t nodesAddr;
    uint32_t nodeCount;
    uint64_t bvh8RootAddr;
};

struct GeometryClusterNodeStorage
{
    uint64_t clusterAddr;
};

struct ClusterStorage
{
    uint64_t positionsAddr;
    uint64_t indicesAddr;
    uint32_t indexCount;
    float3 centroid;
};