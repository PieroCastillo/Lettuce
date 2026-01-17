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
    int idx;
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
    float32_t3 color;
    float32_t2 pos; // NDC: [-1, 1]
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
        r4 * 2.0 - 1.0);

    return outv;
}

struct IndirectBuffer
{
    uint count;
    VkDrawIndirectCommand cmds[256];
};

struct ParticleBuffer
{
    ParticleOut particles[256];
};

struct PushConstants
{
    uint64_t indirectAddress;
    uint64_t particleAddress;
};

[[vk::push_constant]]
PushConstants pc;

[require(compute, spirv_1_6)]
[numthreads(32, 1, 1)]
void compMain(uint3 DTid: SV_DispatchThreadID)
{
    uint globalID = DTid.x;

    if (globalID == 0 || globalID >= 128)
        return;

    Ptr<IndirectBuffer> indirect = reinterpret<Ptr<IndirectBuffer>>(pc.indirectAddress);
    Ptr<ParticleOut> particles = reinterpret<Ptr<ParticleOut>>(pc.particleAddress);

    var particle = generateParticle(globalID);

    float padding = 0.8;
    if (particle.pos.x < -padding || particle.pos.x > padding ||
        particle.pos.y < -padding || particle.pos.y > padding)
    {
        // culled
        return;
    }

    int val;
    InterlockedAdd(indirect->count, 1, val);

    indirect->cmds[globalID] = VkDrawIndirectCommand(3, 1, 0, 0);
    particles[globalID] = particle;
}

[require(vertex, spirv)]
[shader("vertex")]
VSOutput vertexMain(uint vertexID: SV_VulkanVertexID, uint drawID: SV_DrawIndex)
{
    VSOutput out;

    float val = 0.1;
    float2 positions[3] = {
        float2(0.0, -val),
        float2(val, val),
        float2(-val, val)
    };

    Ptr<ParticleOut> particles = reinterpret<Ptr<ParticleOut>>(pc.particleAddress);
    var particle = particles[drawID];

    out.position = float4(positions[vertexID % 3] + particle.pos, 0.0, 1.0);
    out.color = particle.color;

    return out;
}

[require(fragment, spirv)]
[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target
{
    return float4(input.color, 1.0);
}
