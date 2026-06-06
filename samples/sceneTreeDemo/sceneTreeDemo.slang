/*
Created by @PieroCastillo on 2026-06-05
*/

// shader inputs
Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D storageTextures[];

[require(fragment, spirv_1_6)]
[shader("fragment")]
float4 fragMain() : SV_Target
{
    return float4(1, 0, 0, 1);
}