/*
Created by @PieroCastillo on 2025-02-16
*/

const static uint32_t maxInstanceCount = 10000; // I will need to convert this to a constante maybe??

struct VkDrawMeshTasksIndirectCommandEXT
{
    uint32_t groupCountX;
    uint32_t groupCountY;
    uint32_t groupCountZ;
}

struct IndirectBuffer
{
    uint count;
    VkDrawMeshTasksIndirectCommandEXT cmds[maxInstanceCount];
};

struct BVHNodeGPUItem
{
    float3 aabbMin;
    uint32_t leftFirst;
    float3 aabbMax;
    uint32_t count;
};

struct TLASGPUItem
{
    uint32_t rootNodeIdx;
};

struct BLASGPUItem
{
    uint32_t rootNodeIdx;
};

struct BlasLeafMeshletGPUItem
{
    uint32_t meshletGlobalIndex;
};

struct InstanceGPUItem
{
    float3 aabbMin;
    uint32_t meshletIdx;
    float3 aabbMax;
    uint32_t materialDataIdx;
    float4x4 model;
    float4x4 modelInv;
};

// 16 bytes is enough
struct MaterialDataGPUItem
{
    uint32_t materialID;
    uint8_t mmaterialData[15];
};

struct MeshletGPUItem
{
    uint32_t vertexOffset;
    uint32_t triangleOffset;
    uint32_t vertexCount;
    uint32_t triangleCount;
    float3 center;
    float radius;
};

struct VisibilityBufferGPUPixel
{
    uint8_t LOD;
    uint8_t triangleID;
    uint16_t instanceID;
    uint32_t meshletID;
};

struct LightGPUData
{
    float3 color;
    float3 direction;
    float intensity;
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

struct PCCullingPass
{
    uint64_t mvSceneAddress;                   // read
    uint64_t mvTlasNodesAddress;               // read
    uint64_t mvInstancesAddress;               // read
    uint64_t mvVisibilityPassDrawcallsAddress; // write: count+drawcalls
};

struct PCVisibilityPass
{
    uint64_t mvSceneAddress;
    uint64_t mvInstancesAddress;
    uint64_t mvBlasNodesAddress;
    uint64_t mvBlasLeafMeshletIndicesAddress;
    uint64_t mvMeshletsAddress;
    uint64_t mvMaterialDatasAddress;
    uint64_t mvIndexBufferAddress;
    uint64_t mvVertexPosBufferAddress;
    uint64_t mvVertexNorBufferAddress;
    uint64_t mvVertexUVsBufferAddress;
};

struct PCMaterialShadingPass
{
    uint64_t mvSceneAddress; // provides frame index
    uint64_t mvMaterialDatasAddress;
    uint64_t mvIndexBufferAddress;
    uint64_t mvVertexPosBufferAddress;
    uint64_t mvVertexNorBufferAddress;
    uint64_t mvVertexUVsBufferAddress;
};

struct MeshOutput
{
    VisibilityBufferGPUPixel data;
};

struct MeshVisibilityPassPayload
{
};