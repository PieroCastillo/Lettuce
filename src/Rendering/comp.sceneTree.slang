module LettuceRendering;

__include "types.part.slang";

const static uint computeNumThreadsX = 32;

Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D<float4> storageTextures[];

groupshared uint64_t clusterIDs[computeNumThreadsX];
groupshared uint64_t nearestN[computeNumThreadsX];

Ptr<T> loadAddr<T>(uint64_t addr, uint32_t offset = 0)
{
    return reinterpret<Ptr<T>>(addr + (offset * sizeof(T)));
}

uint64_t encodeRelativeOffset(uint64_t id, uint64_t neighborID)
{
    var offset = neighborID - id;
    return abs(offset << 1) | (offset >> 31);
}

uint clz64(uint64_t x)
{
    uint hi = uint(x >> 32);
    uint lo = uint(x & 0xFFFFFFFF);
    return hi != 0 ? firstbithigh(hi) : (lo != 0 ? 32 + firstbithigh(lo) : 64);
}

int delta(uint64_t i, uint64_t j, Ptr<uint64_t> mCodes, uint64_t clusterCount)
{
    if (j < 0 || j >= clusterCount)
        return -1;

    uint64_t codeI = mCodes[i];
    uint64_t codeJ = mCodes[j];

    if (codeI == codeJ)
        return int(clz64(i ^ j)) + 64;

    return int(clz64(codeI ^ codeJ));
}

bool findParentID(uint64_t L, uint64_t R, uint64_t clusterCount, Ptr<uint64_t> mCodes)
{
    int dLeft = delta(L, L - 1, mCodes, clusterCount);
    int dRight = delta(R, R + 1, mCodes, clusterCount);
    return dRight < dLeft;
}

float calculateSAH(float3 aMin1, float3 aMax1, float3 aMin2, float3 aMax2)
{
    float3 mergedMin = min(aMin1, aMin2);
    float3 mergedMax = max(aMax1, aMax2);
    float3 extent = mergedMax - mergedMin;
    return 2.0f * (extent.x * extent.y + extent.y * extent.z + extent.z * extent.x);
}

[require(compute, spirv_1_6)]
[numthreads(computeNumThreadsX, 1, 1)]
void buildCluster([[vk::push_constant]] BuildGeometryClusterPayload payload,
                  uint3 gid: SV_DispatchThreadID, uint3 groupThreadID: SV_GroupThreadID)
{
    uint localIdx = groupThreadID.x;
    uint globalIdx = gid.x;

    if (globalIdx >= payload.clusterBuildCount)
        return;

    var gcluster = loadAddr<GeometryClusterStorage>(payload.dstGeometryClusterAddr);
    var nodes = loadAddr<BVH8NodeStorage>(payload.geometryClusterNodesAddr);

    var tCmd = loadAddr<ClusterBuildIndirectCommand>(payload.clusterBuildsAddr)[globalIdx];
    var tCluster = loadAddr<ClusterStorage>(payload.clusterStorageAddr)[globalIdx];
    var tLeaf = loadAddr<GeometryClusterLeafStorage>(payload.geometryClusterLeavesAddr)[globalIdx];

    // build cluster per leave
    tCluster.centroid = tCmd.bboxMin + 0.5f * (tCmd.bboxMax - tCmd.bboxMin);
    tCluster.indexCount = tCmd.indexCount;
    tCluster.indicesBaseOffset = tCmd.indexBaseOffset;
    tCluster.positionsBaseOffset = tCmd.vertexBaseOffset;
    tLeaf.clusterIdx = globalIdx;

    // h ploc
    /*
    1. calc morton codes and setup other stuff
    barrier()
    2. do outer loop
          inner loop
    barrier()
    3. convert to wide bvh8
    */
}

[require(compute, spirv_1_6)]
[numthreads(computeNumThreadsX, 1, 1)]
void buildPartition([[vk::push_constant]] BuildScenePartitionPayload payload,
                    uint3 gid: SV_DispatchThreadID, uint3 groupThreadID: SV_GroupThreadID)
{
    uint localIdx = groupThreadID.x;
    uint globalIdx = gid.x;

    if (globalIdx >= payload.partitionBuildCount)
        return;

    var partition = loadAddr<ScenePartitionStorage>(payload.dstScenePartitionAddr);
    var nodes = loadAddr<BVH8NodeStorage>(payload.scenePartitionNodesStorageAddr);

    var tCmd = loadAddr<ScenePartitionBuildIndirectCommand>(payload.partitionBuildsAddr)[globalIdx];
    var tLeaf = loadAddr<ScenePartitionLeafStorage>(payload.scenePartitionNodesStorageAddr)[globalIdx];
    var tCluster = loadAddr<GeometryClusterStorage>(payload.geometryClusterStorageAddr)[globalIdx];
    
    
    // h ploc
    /*
    1. calc morton codes and setup other stuff
    barrier()
    2. do outer loop
          inner loop
    barrier()
    3. convert to wide bvh8
    */
}

[require(compute, spirv_1_6)]
[numthreads(computeNumThreadsX, 1, 1)]
void updatePartition([[vk::push_constant]] UpdateScenePartitionPayload payload,
                     uint3 gid: SV_DispatchThreadID, uint3 groupThreadID: SV_GroupThreadID)
{
    uint localIdx = groupThreadID.x;
    uint globalIdx = gid.x;

    if (globalIdx >= payload.partitionUpdateCount)
        return;

    var partition = reinterpret<Ptr<ScenePartitionBuildIndirectCommand>>(payload.dstScenePartitionAddr);
    var cmd = reinterpret<Ptr<ScenePartitionUpdateIndirectCommand>>(payload.partitionUpdatesAddr)[globalIdx];

    // h ploc
    /*
    1. calc morton codes and setup other stuff
    barrier()
    2. do outer loop
          inner loop
    barrier()
    3. convert to wide bvh8
    */
}
