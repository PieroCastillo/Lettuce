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
void pDrawCommandsMain(uint3 dispatchThreadId: SV_DispatchThreadID,
                       uint3 groupThreadId: SV_GroupThreadID,
                       uint3 groupId: SV_GroupID)
{
    var drawCmds = reinterpret<Ptr<DrawCommand>>(pc.drawCommandsAddr);
    var transforms = reinterpret<Ptr<float3x3>>(pc.transformsAddr);
    var implicitGeometries = reinterpret<Ptr<ImplicitGeometryStorage>>(pc.implicitGeometriesAddr);
    var solidColorBrushes = reinterpret<Ptr<SolidColorBrushStorage>>(pc.solidColorBrushesAddr);

    // tile sort by depth
    /*
    // each thread will test a tile
    foreach drawCmd in drawCmds[i, i+(tileSize*tileSize)] and tileDrawCallCount < tileMaxCommands:
        if testIntersection(drawCmd, tile):
            add drawCmd to tileMemory
    */
    // GroupMemoryBarrierWithGroupSync();
    // execute draw commands
    /*
    // each thread will test a pixel
    if (pixel is not in target)
        return;

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
