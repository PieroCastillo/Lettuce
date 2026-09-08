module LettuceRendering;

__include "../shaderShared/types.part.slang";
__include "../shaderShared/materials.part.slang";

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
    float3 baryCoord;
};

struct PSOutput
{
    float4 colorOutput : SV_Target0;
    uint32_t pickOutput : SV_Target1;
};

struct PushConstants
{
    uint64_t sceneViewDataAddr;
    uint64_t indirectCommandsAddr;
    uint64_t positionsBufferAddr;
    uint64_t uvsBufferAddr;
    uint64_t indexBufferAddr;
    uint64_t clusterStorageAddr;
    uint64_t meshStorageAddr;
    uint64_t instanceStorageAddr;
};

Ptr<T> loadAddr<T>(uint64_t addr, uint32_t offset = 0)
{
    return reinterpret<Ptr<T>>(addr + (offset * sizeof(T)));
}

[[vk::push_constant]]
PushConstants pc;

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
    output.color = float3(1);
    output.instanceID = instanceID;
    var vertexIdxMod = vertexID % 3;
    output.baryCoord = float3(vertexIdxMod == 0 ? 1 : 0, vertexIdxMod == 1 ? 1 : 0, vertexIdxMod == 2 ? 1 : 0);

    return output;
}

[require(fragment, spirv_1_6)]
[shader("fragment")]
PSOutput fragMain(VSOutput input)
{
    PSOutput output;
    output.colorOutput = float4(input.color, 1);
    output.pickOutput = input.instanceID + 1;

    return output;
}