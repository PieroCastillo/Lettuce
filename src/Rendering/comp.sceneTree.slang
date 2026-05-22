module LettuceRendering;

__include "types.part.slang";

const static uint computeNumThreadsX = 32;

// shader inputs
Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D<float4> storageTextures[];

[require(compute, spirv_1_6)]
[numthreads(computeNumThreadsX, 1, 1)]
void buildCluster([[vk::push_constant]]BuildGeometryClusterPayload payload,
                    uint3 gid: SV_DispatchThreadID, uint3 groupThreadID: SV_GroupThreadID)
{
    uint localIdx = groupThreadID.x;
    uint globalIdx = gid.x;

    if (globalIdx >= payload.clusterCount)
        return;
}

[require(compute, spirv_1_6)]
[numthreads(computeNumThreadsX, 1, 1)]
void buildPartition([[vk::push_constant]]BuildScenePartitionPayload payload,
                    uint3 gid: SV_DispatchThreadID, uint3 groupThreadID: SV_GroupThreadID)
{
    uint localIdx = groupThreadID.x;
    uint globalIdx = gid.x;

    if (globalIdx >= payload.instanceCount)
        return;
}

[require(compute, spirv_1_6)]
[numthreads(computeNumThreadsX, 1, 1)]
void updatePartition([[vk::push_constant]]UpdateScenePartitionPayload payload,
                    uint3 gid: SV_DispatchThreadID, uint3 groupThreadID: SV_GroupThreadID)
{
    uint localIdx = groupThreadID.x;
    uint globalIdx = gid.x;

    if (globalIdx >= payload.instanceCount)
        return;

    ScenePartitionUpdateIndirectCommand cmd;
}