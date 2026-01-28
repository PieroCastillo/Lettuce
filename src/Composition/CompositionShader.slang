/*
Created by @PieroCastillo on 2026-01-27
*/

// shader inputs
Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D<float4> storageTextures[];

struct PushConstants
{
    uint64_t indirectAddress;
    uint64_t particleAddress;
};

[[vk::push_constant]]
PushConstants pc;

[require(compute, spirv_1_6)]
[numthreads(32, 1, 1)]
void queuedAnimationsPassMain(uint3 DTid: SV_DispatchThreadID)
{
}

[require(compute, spirv_1_6)]
[numthreads(32, 1, 1)]
void animationsPassMain(uint3 DTid: SV_DispatchThreadID)
{
}

[require(compute, spirv_1_6)]
[numthreads(32, 1, 1)]
void visualsPassMain(uint3 DTid: SV_DispatchThreadID)
{
}

[require(compute, spirv_1_6)]
[numthreads(32, 1, 1)]
void brushesPassMain(uint3 DTid: SV_DispatchThreadID)
{
}

[require(compute, spirv_1_6)]
[numthreads(32, 1, 1)]
void lightsPassMain(uint3 DTid: SV_DispatchThreadID)
{
}

[require(compute, spirv_1_6)]
[numthreads(32, 1, 1)]
void effectsPassMain(uint3 DTid: SV_DispatchThreadID)
{
}
