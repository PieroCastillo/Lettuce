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
    uint64_t positionsBufferAddr;
    uint64_t indexBufferAddr;
    uint64_t clusterStorageAddr;
    uint64_t meshStorageAddr;
    uint64_t instanceStorageAddr;
};

Ptr<T> loadAddr<T>(uint64_t addr, uint32_t offset = 0)
{
    return reinterpret<Ptr<T>>(addr + (offset * sizeof(T)));
}

uint hash_u32(uint x)
{
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

float hash_to_float01(uint x)
{
    return (hash_u32(x) & 0x00FFFFFF) / 16777216.0;
}

float3 make_random_color(uint id)
{
    float r0 = hash_to_float01(id * 3u + 0u);
    float r1 = hash_to_float01(id * 3u + 1u);
    float r2 = hash_to_float01(id * 3u + 2u);
    return float3(r0, r1, r2);
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
    if (idx >= sceneViewData.instanceCount)
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
    uint cmdIdx;
    InterlockedAdd(drawCmdStorage.count, 1, cmdIdx);
    drawCmdStorage.commands[cmdIdx] = drawCmd;
}

[shader("vertex")]
VSOutput vertMain(uint vertexID: SV_VulkanVertexID, uint instanceID: SV_DrawIndex, uint gClusterIdx: SV_VulkanInstanceID)
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

    var worldPos = mul(instance.localTransform, float4(position, 1.0));
    output.position = mul(sceneViewData.viewProj, float4(worldPos.xyz, 1.0));
    output.color = 
    make_random_color(cluster.clusterID);

    return output;
}

[require(fragment, spirv_1_6)]
[shader("fragment")]
float4 fragMain(VSOutput input, uint4 fragCoord: SV_Position) : SV_Target
{
    return float4(input.color, 1);
}
