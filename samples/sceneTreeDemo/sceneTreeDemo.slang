/*
Created by @PieroCastillo on 2026-06-05
*/

// shader inputs
Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D storageTextures[];

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

struct VSOutput
{
    float4 position : SV_Position;
    float3 color : COLOR0;
};

struct SceneData
{
    float4x4 viewProj;
};

struct ClusterBuildIndirectCommand
{
    float3 bboxMin;
    float3 bboxMax;
    float4 cone;
    float lodError;
    uint32_t clusterID;
    uint32_t geometryIndex;
    uint32_t triangleOffset;
    uint32_t triangleCount;
    uint32_t vertexOffset;
    uint32_t vertexCount;
    uint32_t vertexStride;
};

struct VertexL
{
    float3 pos;
    float3 norm;
    float2 texCoord0;
};

struct PushConstants
{
    uint64_t sceneDataAddr;
    uint64_t clusterBuildCmdsAddr;
    uint64_t triangleBufferAddr;
    uint64_t vertexBufferAddr;
};

[[vk::push_constant]]
PushConstants pc;

[shader("vertex")]
VSOutput vertMain(uint vertexID: SV_VulkanVertexID, uint drawID: SV_DrawIndex)
{
    VSOutput output;

    var scene = reinterpret<Ptr<SceneData>>(pc.sceneDataAddr)[0];
    var cmd = reinterpret<Ptr<ClusterBuildIndirectCommand>>(pc.clusterBuildCmdsAddr)[drawID];
    var idx = reinterpret<Ptr<uint8_t>>(pc.triangleBufferAddr)[cmd.triangleOffset + vertexID];
    var vertex = reinterpret<Ptr<VertexL>>(pc.vertexBufferAddr)[cmd.vertexOffset + idx];

    output.position = mul(scene.viewProj, float4(vertex.pos, 1.0f));
    output.color = make_random_color(cmd.clusterID);

    return output;
}

[require(fragment, spirv_1_6)]
[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target
{
    return float4(input.color, 1);
}
