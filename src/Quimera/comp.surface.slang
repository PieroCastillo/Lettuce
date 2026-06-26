module LettuceQuimera;

__include "funcs.part.slang";
__include "types.part.slang";

Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D<float4> storageTextures[];

const static uint computeNumThreadsX = 32;
const static uint tileSize = 16;
const static uint tileMaxCommands = 32;

[[vk::push_constant]]
PushConstants pc;

groupshared uint32_t tileDrawCallCount;
groupshared TileMemory tileMemory[tileMaxCommands];

[require(compute, spirv_1_6)]
[numthreads(tileSize, tileSize, 1)]
void pDrawCommandsMain(uint3 gid: SV_DispatchThreadID,
                       uint3 lid: SV_GroupThreadID,
                       uint3 groupId: SV_GroupID)
{
    var surfaceData = reinterpret<Ptr<SurfaceData>>(pc.surfaceDataAddr)[0];
    var drawCmds = reinterpret<Ptr<DrawCommand>>(pc.drawCommandsAddr);
    var transforms = reinterpret<Ptr<float3x3>>(pc.transformsAddr);
    var implicitGeometries = reinterpret<Ptr<ImplicitGeometryStorage>>(pc.implicitGeometriesAddr);
    var solidColorBrushes = reinterpret<Ptr<SolidColorBrushStorage>>(pc.solidColorBrushesAddr);

    if (gid.x >= surfaceData.width || gid.y >= surfaceData.height)
        return;

    if (lid.x == 0 && lid.y == 0)
        tileDrawCallCount = 0;

    var color = float4(0.5, 0.5, 0.5, 1);

    // tile sort by depth and intersection
    uint localIndex = gid.y * tileSize + gid.x;

    for (uint k = 0; k < surfaceData.drawCommandCount; k += tileSize * tileSize)
    {
        uint i = k + localIndex;

        if (i < surfaceData.drawCommandCount)
        {
            uint slot;
            InterlockedAdd(tileDrawCallCount, 1, slot);

            if (slot < tileMaxCommands)
            {
                tileMemory[slot].drawCommandId = i;
                color = float4(1, 0, 0, 1);
            }
            else
            {
                break;
            }
        }
    }

    GroupMemoryBarrierWithGroupSync();

    // clamp tileDrawCallCount
    var tileDrawCmdCount = min(tileDrawCallCount, tileMaxCommands);

    // execute draw commands
    // each thread will test a pixel
    /// TODO: convert from local coords to screen coords 
    for (uint j = 0; j < tileDrawCmdCount; ++j)
    {
        var drawCmd = drawCmds[tileMemory[j].drawCommandId];
        uint brushHeap, geometryHeap, effectHeap;
        bool ignoreTransform, clipped;
        UnpackDrawCommandFlags(drawCmd.flags, geometryHeap, brushHeap, effectHeap, ignoreTransform, clipped);

        if (brushHeap == 0 && geometryHeap == 0)
        {
            var brush = solidColorBrushes[drawCmd.brushIdx];
            var geometry = implicitGeometries[drawCmd.geometryIdx];
            var transform = transforms[drawCmd.transformIdx]; // inverses
            var pixelPos = float2(gid.xy);

            // if(!ignoreTransform)
            // pixelPos = mul(transform, float3(pixelPos, 1)).xy;

            if(IntersectImplicit(uint2(pixelPos), geometry, transform))
            {
                color = brush.color;
            }

            if (brush.color.a >= 1.0)
                break;
        }
    }

    storageTextures[0][uint2(gid.xy)] = color;
    /*
    // each thread will test a pixel

    for each drawCmd in tileMemory:
        load brush
        load geometry
        load transform
        if (test(TransformedGeometry, pixel))
            target[pixel] = shade(pixel, brush)
        if (fullOpaque(brush))
            break;
    */
}
