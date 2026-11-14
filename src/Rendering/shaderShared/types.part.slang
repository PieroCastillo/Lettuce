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
    float4x4 invViewProj;
    float3 cameraPosition;
    uint32_t instanceCount;
    float3 cameraForward;
    float _padding;
};

struct MaterialShadingData
{
    uint32_t depthTextureIdx;
    uint32_t gbufferTextureIdx;
};

struct ClusterStorage
{
    float4 cone;
    float4 aabbMin;
    float4 aabbMax;
    float4 centroid;

    uint32_t materialHeap;
    uint32_t materialID;

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