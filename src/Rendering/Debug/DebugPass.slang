module LettuceRendering;

__include "types.part.slang";

const static uint computeNumThreadsX = 32;
const static uint maxTriangles = 126;
const static uint maxIndices = maxTriangles * 3;

Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D<float4> storageTextures[];

struct VSOutput
{
    float4 position : SV_Position;
    float3 color : COLOR0;
    nointerpolation uint32_t instanceID;
};

struct PushConstants
{
    uint64_t sceneViewDataAddr;
    uint64_t indirectCommandsAddr;
    uint64_t clusterStorageAddr;
    uint64_t meshStorageAddr;
    uint64_t instanceStorageAddr;
    uint64_t positionsBufferAddr;
    uint64_t normalsBufferAddr;
    uint64_t tangentsBufferAddr;
    uint64_t texCoordsBufferAddr;
    uint64_t indexBufferAddr;
};

Ptr<T> loadAddr<T>(uint64_t addr, uint32_t offset = 0)
{
    return reinterpret<Ptr<T>>(addr + (offset * sizeof(T)));
}

[[vk::push_constant]]
PushConstants pc;

[require(compute, spirv_1_6)]
[numthreads(computeNumThreadsX, 1, 1)]
void buildCommandsMain(uint3 gid: SV_DispatchThreadID,
              uint3 lid: SV_GroupThreadID,
              uint3 groupId: SV_GroupID)
{
    var idx = gid.x;

    // thread check
    var sceneViewData = loadAddr<SceneViewData>(pc.sceneViewDataAddr)[0];
    if(idx >= sceneViewData.instanceCount)
    {
        return;
    }

    // in
    var instance = loadAddr<InstanceStorage>(pc.instanceStorageAddr)[idx];
    var mesh = loadAddr<MeshStorage>(pc.meshStorageAddr)[instance.meshIdx];
    var cluster = loadAddr<ClusterStorage>(pc.clusterStorageAddr, mesh.clusterOffset)[0];
    // out
    var drawCmdStorage = loadAddr<IndirectCommandStorage>(pc.indirectCommandsAddr);

    if (idx == 0)
    {
        drawCmdStorage.count = 0;
    }
    GroupMemoryBarrierWithGroupSync();

    VkDrawIndirectCommand drawCmd;
    drawCmd.vertexCount = maxIndices; // full pulling
    drawCmd.instanceCount = mesh.clusterCount;
    drawCmd.firstVertex = 0;
    drawCmd.firstInstance = mesh.clusterOffset;

    // write output
    InterlockedAdd(drawCmdStorage.count, 1);
    drawCmdStorage.commands[idx] = drawCmd;
}

[shader("vertex")]
VSOutput vertMain(uint vertexID: SV_VulkanVertexID, uint instanceID: SV_DrawIndex, uint gClusterIdx : SV_InstanceID)
{
    // in
    var sceneViewData = loadAddr<SceneViewData>(pc.sceneViewDataAddr)[0];
    var instance = loadAddr<InstanceStorage>(pc.instanceStorageAddr)[instanceID];
    var cluster = loadAddr<ClusterStorage>(pc.clusterStorageAddr)[gClusterIdx];

    // out
    VSOutput output;

    if (vertexID >= cluster.triangleCount)
    {
        output.position = float4(0, 0, 0, 1);
        output.color = float3(0, 0, 0);
        return output;
    }

    var index = loadAddr<uint8_t>(pc.indexBufferAddr, cluster.trianglesBaseOffset)[vertexID];
    var position = loadAddr<float3>(pc.positionsBufferAddr, cluster.positionsBaseOffset)[index];
    
    output.position = mul(sceneViewData.viewProj, float4(position, 1.0));
    output.color = float3(1.0);

    return output;
}

[require(fragment, spirv_1_6)]
[shader("fragment")]
float4 fragMain(VSOutput input, uint4 fragCoord: SV_Position) : SV_Target
{
    return float4(input.color, 1);
}