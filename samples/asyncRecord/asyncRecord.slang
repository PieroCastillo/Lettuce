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

struct ParticleOut
{
    float32_t3 color;
    float32_t2 pos; // NDC: [-1, 1]
};

struct ParticleBuffer
{
    uint32_t idx;
    ParticleOut particles[2];
};

struct PushConstants
{
    uint64_t particleAddress;
};

[[vk::push_constant]]
PushConstants pc;

[require(vertex, spirv)]
[shader("vertex")]
VSOutput vertexMain(uint vertexID: SV_VulkanVertexID)
{
    VSOutput out;

    float val = 0.5;
    float2 positions[3] = {
        float2(0.0, -val),
        float2(val, val),
        float2(-val, val)
    };

    Ptr<ParticleBuffer> particles = reinterpret<Ptr<ParticleBuffer>>(pc.particleAddress);
    var particle = particles->particles[particles->idx];

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