module LettuceQuimera;

__include "types.part.slang";

const static uint computeNumThreadsX = 32;
const static uint tileSize = 16;

[require(compute, spirv_1_6)]
[numthreads(computeNumThreadsX, 1, 1)]
void pPrepareMain()
{
}

[require(compute, spirv_1_6)]
[numthreads(tileSize, tileSize, 1)]
void pTileBinningMain()
{
}

[require(compute, spirv_1_6)]
[numthreads(tileSize, tileSize, 1)]
void pBrushesMain()
{
}