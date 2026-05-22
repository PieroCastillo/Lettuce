module LettuceRendering;

__include "types.part.slang";

const static uint computeNumThreadsX = 32;

// shader inputs
Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D<float4> storageTextures[];

[require(compute, spirv_1_6)]
[numthreads(computeNumThreadsX, 1, 1)]
void buildCluster(uint3 gid: SV_DispatchThreadID, uint3 groupThreadID: SV_GroupThreadID)
{
    uint localIdx = groupThreadID.x;
    uint globalIdx = gid.x;
}

[require(compute, spirv_1_6)]
[numthreads(computeNumThreadsX, 1, 1)]
void buildPartition(uint3 gid: SV_DispatchThreadID, uint3 groupThreadID: SV_GroupThreadID)
{
    uint localIdx = groupThreadID.x;
    uint globalIdx = gid.x;
}

[require(compute, spirv_1_6)]
[numthreads(computeNumThreadsX, 1, 1)]
void updatePartition(uint3 gid: SV_DispatchThreadID, uint3 groupThreadID: SV_GroupThreadID)
{
    uint localIdx = groupThreadID.x;
    uint globalIdx = gid.x;

    ScenePartitionUpdateIndirectCommand cmd;
}