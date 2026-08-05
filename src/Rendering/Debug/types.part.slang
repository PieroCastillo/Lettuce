implementing LettuceRendering;

struct VkDrawIndirectCommand
{
    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
    uint32_t firstInstance;
};

struct IndirectCommandStorage
{
    uint32_t count;
    VkDrawIndirectCommand commands[];
};

struct SceneViewData
{
    float4x4 viewProj;
    uint32_t instanceCount;
};

struct ClusterStorage
{
    uint32_t materialHeap;
    uint32_t materialID;

    float4 cone;
    float3 aabbMin;
    float3 aabbMax;
    float3 centroid;

    uint32_t positionsBaseOffset;
    uint32_t positionsCount;
    uint32_t normalsBaseOffset;
    uint32_t normalsCount;
    uint32_t tangentsBaseOffset;
    uint32_t tangentsCount;
    uint32_t texCoords0BaseOffset;
    uint32_t texCoords0Count;

    uint32_t trianglesBaseOffset;
    uint32_t triangleCount;
    uint32_t clusterID;
};

struct MeshStorage
{
    float3 aabbMin;
    float3 aabbMax;
    uint32_t clusterOffset;
    uint32_t clusterCount;
};

struct InstanceStorage
{
    float4x4 localTransform;
    uint32_t meshIdx;
};