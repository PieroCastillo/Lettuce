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
// update visual/effects/lights/brushes properties
void animationsPassMain(uint3 DTid: SV_DispatchThreadID)
{
}

[require(compute, spirv_1_6)]
[numthreads(32, 1, 1)]
void tilesPassMain(uint3 DTid: SV_DispatchThreadID)
{
}

[require(compute, spirv_1_6)]
[numthreads(16, 16, 1)]
void rasterPass(uint3 DTid: SV_DispatchThreadID)
{
}

/*
for pixel (x,y):
    finalColor = background

    for element in drawOrder:
        if !inside(element, x,y): continue

        color = evalBrush(element.brush)

        for light in affectingLights:
            color = applyLight(light, color)

        for effect in element.effects:
            color = applyEffect(effect, color)

        finalColor = alphaBlend(color, finalColor)

        if finalColor.a == 1:
            break

*/