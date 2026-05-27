implementing LettuceRendering;

struct BuildGeometryClusterPayload
{
    uint64_t dstGeometryClusterAddr;
    uint64_t clusterBuildsAddr;
    uint64_t clusterStorageAddr;
    uint64_t geometryClusterLeavesAddr;
    uint64_t geometryClusterNodesAddr;
    uint64_t scratchAddr;
    
    uint64_t positionsAddr;
    uint64_t indicesAddr;

    uint32_t clusterBuildCount;
    uint32_t positionStride;
    uint32_t flags;
};

struct BuildScenePartitionPayload
{
    uint64_t dstScenePartitionAddr;
    uint64_t partitionBuildsAddr;
    uint64_t geometryClusterStorageAddr;
    uint64_t scenePartitionLeavesStorageAddr;
    uint64_t scenePartitionNodesStorageAddr;
    uint64_t scratchAddr;

    uint32_t partitionBuildCount;
};

struct UpdateScenePartitionPayload
{
    uint64_t dstScenePartitionAddr;
    uint64_t partitionUpdatesAddr;
    uint64_t geometryClusterStorageAddr;
    uint64_t scenePartitionLeavesStorageAddr;
    uint64_t scenePartitionNodesStorageAddr;
    uint64_t scratchAddr;

    uint32_t partitionUpdateCount;
};

struct ClusterBuildIndirectCommand
{
    float3 bboxMin;
    float3 bboxMax;
    float4 cone;
    float lodError;
    uint32_t clusterID;
    uint32_t indexBaseOffset;
    uint32_t indexCount;
    uint32_t vertexBaseOffset;
    uint32_t vertexCount;
};

// each build is a new instance into the partition
struct ScenePartitionBuildIndirectCommand
{   
    float4x4 transform;
    uint32_t instanceIndex;
    uint32_t instanceContributionToHitGroupIndex;
    uint32_t geometryClusterIdx;
    uint32_t instanceMask;
    uint32_t flags;
};

struct ScenePartitionUpdateIndirectCommand
{
    float4x4 transform;
    uint32_t instanceIndex;
    uint32_t instanceContributionToHitGroupIndex;
};

// for kernel launch only
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

// for Geometry Cluster & Scene Partition
struct BVH8NodeStorage
{
    float3 aabbMin[8];
    float3 aabbMax[8];

    uint32_t parentNodeIdx[8];
    uint32_t childBaseIdx[8];
    uint32_t childCount[8];
    uint32_t validChildMask[8];
};

// node leaf storages
struct ScenePartitionLeafStorage
{
    uint32_t geometryClusterIdx;
    float4x4 transform;
    float4 cone;
};

struct GeometryClusterLeafStorage
{
    uint32_t clusterIdx;
};

// handle storages
struct ScenePartitionStorage
{
    uint32_t scenePartitionLeavesBaseOffset;
    uint32_t scenePartitionNodesBaseOffset;

    uint32_t leafCount;
    uint32_t nodeCount;
    uint32_t bvh8RootIdx;
};

struct GeometryClusterStorage
{
    uint32_t geometryClusterLeavesBaseOffset;
    uint32_t geometryClusterNodesBaseOffset;
    
    uint32_t leafCount;
    uint32_t nodeCount;
    uint32_t bvh8RootIdx;
};

struct ClusterStorage
{
    float4 cone;
    float3 bboxMin;
    float3 bboxMax;
    float3 centroid;
    uint32_t positionsBaseOffset;
    uint32_t indicesBaseOffset;
    uint32_t clusterID;
    uint32_t indexCount;
    float lodError;
};