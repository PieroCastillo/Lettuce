/*
Created by @PieroCastillo on 2025-10-31
*/

// shader inputs
Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D<float4> storageTextures[];

struct VSOutput
{
    float4 position : SV_Position;
    float3 color : COLOR0;
};

struct VkDrawIndirectCommand
{
    uint vertexCount;
    uint instanceCount;
    uint firstVertex;
    uint firstInstance;
};

struct ParticleOut
{
    float3 color;
    float2 pos; // NDC: [-1, 1]
};

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

ParticleOut generateParticle(uint tid_x)
{
    ParticleOut outv;

    float r0 = hash_to_float01(tid_x * 3u + 0u);
    float r1 = hash_to_float01(tid_x * 3u + 1u);
    float r2 = hash_to_float01(tid_x * 3u + 2u);
    float r3 = hash_to_float01(tid_x * 3u + 3u);
    float r4 = hash_to_float01(tid_x * 3u + 4u);

    outv.color = float3(r0, r1, r2);

    outv.pos = float2(
        r3 * 2.0 - 1.0,
        r4 * 2.0 - 1.0
    );

    return outv;
}

[require(compute, spirv)]
[numthreads(32, 1, 1)]
void compMain(uint3 DTid: SV_DispatchThreadID)
{
    uint globalID = DTid.x;
}

[require(vertex, spirv)]
[shader("vertex")]
VSOutput vertexMain(uint vertexID: SV_VulkanVertexID)
{
    VSOutput out;

    float2 positions[3] = {
        float2(0.0, -0.75),
        float2(0.75, 0.75),
        float2(-0.75, 0.75)
    };

    out.position = float4(positions[vertexID % 3], 0.0, 1.0);

    float3 colors[3] = {
        float3(1.0, 0.0, 0.0),
        float3(0.0, 1.0, 0.0),
        float3(0.0, 0.0, 1.0)
    };
    out.color = colors[vertexID % 3];

    return out;
}

[require(fragment, spirv)]
[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target
{
    return float4(input.color, 1.0);
}