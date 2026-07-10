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

groupshared uint32_t tileSortZA[tileMaxCommands];
groupshared uint32_t tileSortZB[tileMaxCommands];

groupshared uint32_t tileSortIdA[tileMaxCommands];
groupshared uint32_t tileSortIdB[tileMaxCommands];

[require(compute, spirv_1_6)]
[numthreads(tileSize, tileSize, 1)]
void pDrawCommandsMain(uint3 gid: SV_DispatchThreadID,
                       uint3 lid: SV_GroupThreadID,
                       uint3 groupId: SV_GroupID)
{
    var surfaceData = reinterpret<Ptr<SurfaceData>>(pc.surfaceDataAddr)[0];
    var transforms = reinterpret<Ptr<float3x3>>(pc.scratchTransformsAddr);

    var drawCmds = reinterpret<Ptr<DrawCommand>>(pc.drawCommandsAddr);
    var layouts = reinterpret<Ptr<LayoutStorage>>(pc.layoutsAddr);
    var implicitGeometries = reinterpret<Ptr<ImplicitGeometryStorage>>(pc.implicitGeometriesAddr);
    var solidColorBrushes = reinterpret<Ptr<SolidColorBrushStorage>>(pc.solidColorBrushesAddr);
    
    const uint globalIdx = gid.x + (gid.y * surfaceData.width);
    const uint localIdx = lid.x + (lid.y * tileSize);
    const uint tileXCount = (surfaceData.width + 15) / 16;
    const uint tileYCount = (surfaceData.height + 15) / 16;
    const uint totalThreadsPerFrame = tileXCount * tileYCount * tileSize * tileSize;
    const bool inBounds = gid.x < surfaceData.width && gid.y < surfaceData.height;

    // shared mem reset per tile
    if (lid.x == 0 && lid.y == 0)
        tileDrawCallCount = 0;

    // we need to calculate transforms at global memory
    for (uint layoutIdx = globalIdx; layoutIdx < surfaceData.layoutCount; layoutIdx += totalThreadsPerFrame)
    {
        var layoutT = layouts[layoutIdx];
        transforms[layoutIdx] = InverseTransform2D(layoutT.position, layoutT.scale, layoutT.skew, layoutT.anchorPoint, layoutT.rotation);
    }

    GroupMemoryBarrierWithGroupSync();

    var color = float4(0.5, 0.5, 0.5, 1);

    // tile sort by depth and intersection
    for (uint k = 0; k < surfaceData.drawCommandCount; k += tileSize * tileSize)
    {
        uint i = k + localIdx;

        if (i < surfaceData.drawCommandCount)
        {
            uint slot;
            InterlockedAdd(tileDrawCallCount, 1, slot);

            if (slot < tileMaxCommands)
            {
                tileMemory[slot].drawCommandId = i;
                continue;
            }
            break;
        }
    }

    GroupMemoryBarrierWithGroupSync();

    uint tileDrawCmdCount = min(tileDrawCallCount, tileMaxCommands);

    if (localIdx < tileDrawCmdCount && localIdx < tileMaxCommands)
    {
        uint drawCommandId = tileMemory[localIdx].drawCommandId;

        tileSortIdA[localIdx] = drawCommandId;
        tileSortZA[localIdx] = drawCmds[drawCommandId].zOrder;
    }
    else if (localIdx < tileMaxCommands)
    {
        tileSortIdA[localIdx] = 0xFFFFFFFFu;
        tileSortZA[localIdx] = 0u;
    }

    GroupMemoryBarrierWithGroupSync();

    uint stage = 0;
    for (uint sequenceSize = 2; sequenceSize <= tileMaxCommands; sequenceSize <<= 1)
    {
        for (uint compareDistance = sequenceSize >> 1; compareDistance > 0; compareDistance >>= 1)
        {
            if (localIdx < tileMaxCommands)
            {
                uint partner = localIdx ^ compareDistance;

                bool comp = (stage & 1u) == 0u;

                var ownId = comp ? tileSortIdA[localIdx] : tileSortIdB[localIdx];
                var ownZ = comp ? tileSortZA[localIdx] : tileSortZB[localIdx];

                var partnerId = comp ? tileSortIdA[partner] : tileSortIdB[partner];
                var partnerZ = comp ? tileSortZA[partner] : tileSortZB[partner];

                bool ownBeforePartner = ownZ > partnerZ || (ownZ == partnerZ && ownId < partnerId);
                bool partnerBeforeOwn = partnerZ > ownZ || (partnerZ == ownZ && partnerId < ownId);
                bool ownIsLowerLane = localIdx < partner;
                bool forward = (localIdx & sequenceSize) == 0u;
                bool wantEarlier = forward ? ownIsLowerLane : !ownIsLowerLane;
                bool keepOwn = wantEarlier ? !partnerBeforeOwn : !ownBeforePartner;

                uint resultId = keepOwn ? ownId : partnerId;
                uint resultZ = keepOwn ? ownZ : partnerZ;

                if ((stage & 1u) == 0u)
                {
                    tileSortIdB[localIdx] = resultId;
                    tileSortZB[localIdx] = resultZ;
                }
                else
                {
                    tileSortIdA[localIdx] = resultId;
                    tileSortZA[localIdx] = resultZ;
                }
            }

            GroupMemoryBarrierWithGroupSync();
            ++stage;
        }
    }

    // 15 iterations
    if (localIdx < tileDrawCmdCount)
    {
        tileMemory[localIdx].drawCommandId = tileSortIdB[localIdx];
    }

    GroupMemoryBarrierWithGroupSync();

    // execute draw commands
    // each thread will test a pixel
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
            var transform = transforms[drawCmd.layoutIdx]; // inverses
            var pixelPos = float2(gid.xy);

            // if (!ignoreTransform)

            if (IntersectImplicit(uint2(pixelPos), geometry, transform))
            {
                color = brush.color;

                if (brush.color.a >= 1.0)
                    break;
            }
        }
    }

    if (inBounds)
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
