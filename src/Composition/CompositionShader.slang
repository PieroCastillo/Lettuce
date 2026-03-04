/*
Created by @PieroCastillo on 2026-01-27
*/

#include "CompositionStructs.slang"

// soon these will be converted to specialization constants
const static uint tileSize = 16;
const static uint maxElementsPerTile = 64;
const static uint computeNumThreadsX = 32;
const static uint maxVisualsPerQueue = 256;

// shader inputs
Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D<float4> storageTextures[];

struct PushConstants
{
    uint64_t frameDataAddress;
    uint64_t animationsDataAddress;
    uint64_t visualsDataAddress;
    uint64_t culledVisualsIDsAddress;
};

[[vk::push_constant]]
PushConstants pc;

bool intersects2D(float2 visualPos, float2 visualSize, float2 tilePos, float2 tileSize)
{
    // AABB intersection test
    return (visualPos.x < tilePos.x + tileSize.x) && (visualPos.x + visualSize.x > tilePos.x) &&
           (visualPos.y < tilePos.y + tileSize.y) && (visualPos.y + visualSize.y > tilePos.y);
}

[require(compute, spirv_1_6)]
[numthreads(computeNumThreadsX, 1, 1)]
// update visual/effects/lights/brushes properties
void animationsPassMain(uint3 DTid: SV_DispatchThreadID)
{
}

[require(compute, spirv_1_6)]
[numthreads(computeNumThreadsX, 1, 1)]
void cullingPassMain(uint3 gid: SV_DispatchThreadID)
{
    var visualsBuffer = reinterpret<Ptr<VisualGPUData>>(pc.visualsDataAddress);
    var culledVisualsBuffer = reinterpret<Ptr<uint32_t>>(pc.culledVisualsIDsAddress);
    var frameData = reinterpret<Ptr<FrameGPUData>>(pc.frameDataAddress);

    var visualIdx = gid.x;
    var visual = visualsBuffer[visualIdx];

    if (intersects2D(visual.Position, visual.Size, float2(0, 0), float2(frameData.viewportSize.x, frameData.viewportSize.y)))
    {
        // Atomically add the visual index to the culled visuals buffer
        uint idx = 0;
        InterlockedAdd(culledVisualsBuffer[0], 1, idx);
        culledVisualsBuffer[idx + 1] = visualIdx;
    }
}

groupshared uint visualTileQueue[maxVisualsPerQueue];
groupshared uint visualTileQueueCount;

[require(compute, spirv_1_6)]
[numthreads(computeNumThreadsX, 1, 1)]
// record visuals
void tilesPassMain(uint3 globalThreadID: SV_DispatchThreadID, uint3 localThreadID: SV_GroupThreadID, uint3 groupID: SV_GroupID)
{
    var visualsBuffer = reinterpret<Ptr<VisualGPUData>>(pc.culledVisualsIDsAddress);
    // cull visuals
    /*
        for each visual in visuals:
            if visual is into tile:
                visualTileQueue.add(visual)
    */

    GroupMemoryBarrierWithGroupSync();

    // raster visuals to visibility target
}

[require(compute, spirv_1_6)]
[numthreads(tileSize, tileSize, 1)]
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