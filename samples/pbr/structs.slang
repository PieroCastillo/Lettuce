/*
Created by @PieroCastillo on 2025-02-16
*/

const static uint32_t maxInstanceCount = 10000; // I will need to convert this to a constante maybe??
const static uint32_t meshletVertexCount = 64;
const static uint32_t meshletTriangleCount = 126;
const static uint32_t computeNumthreadsSizeX = 32;
const static uint32_t tileSizeX = 16;
const static uint32_t tileSizeY = 16;

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

struct TaskRecord
{
    uint32_t instanceID;
};

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
    float radius;
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

struct PCCullingPass
{
    uint64_t mvSceneAddress;                   // read
    uint64_t mvTlasNodesAddress;               // read
    uint64_t mvInstancesAddress;               // read
    uint64_t mvVisibilityPassDrawcallsAddress; // write: count+drawcalls
    uint64_t mvTaskRecordsAddress;             // write
};

struct PCVisibilityPass
{
    uint64_t mvSceneAddress;
    uint64_t mvInstancesAddress;
    uint64_t mvBlasNodesAddress;
    uint64_t mvMeshletsAddress;
    uint64_t mvTaskRecordsAddress;
    
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

struct MeshVertexOutput
{
    float4 pos : SV_Position;
    VisibilityBufferGPUPixel data;
};

struct MeshVisibilityPassPayload
{
    uint16_t instanceID;
    uint meshletID;
};