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

struct VertexOutput
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
    // uint64_t clusterBuildCmdsAddr;
    // uint64_t triangleBufferAddr;
    // uint64_t vertexBufferAddr;
};

[shader("vertex")]
VertexOutput vertMain([[vk::push_constant]] PushConstants pc, uint vertexID: SV_VulkanVertexID, uint drawID: SV_DrawIndex)
{
    VertexOutput output;

    var scene = reinterpret<Ptr<SceneData>>(pc.sceneDataAddr)[0];
    // var cmd = loadAddr<ClusterBuildIndirectCommand>(pc.clusterBuildCmdsAddr)[drawID];
    // var idx = loadAddr<uint8_t>(pc.triangleBufferAddr)[cmd.triangleOffset + vertexID];
    // var vertex = loadAddr<VertexL>(pc.vertexBufferAddr)[cmd.vertexOffset + idx];

    float2 positions[3] =
        {
            float2(-0.1f, 0.1f),
            float2(0.0f, 0.1f),
            float2(0.1f, -0.1f),
        };

    output.position = float4(positions[vertexID % 3], 0.0f, 1.0f);
    // output.position = mul(scene.viewProj, float4(vertex.pos, 1.0f));
    // output.color = float3(cmd.clusterID / 100.0, 0, 0);
    output.color = float3(1, 0, 0); // vertex.norm;

    return output;
}

[require(fragment, spirv_1_6)]
[shader("fragment")]
float4 fragMain(VertexOutput input) : SV_Target
{
    return float4(input.color, 1);
}