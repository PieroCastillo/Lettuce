module LettuceRendering;

__include "types.part.slang";

const static uint computeNumThreadsX = 32;

// shader inputs
Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D<float4> storageTextures[];

groupshared BVH2NodeStorage bvh2nodes[1024]; // BVH2 nodes, first entries are set to leaf bounding boxes
groupshared uint mortonCodes[1024];          // Morton codes sorted in ascending order
groupshared uint mcClusterIDs[1024];         // cluster indices extracted from sorted Morton codes
groupshared uint64_t parentIDs[1024];        // BVH2parentIDs, initialized to −1

groupshared uint64_t clusterIDs[computeNumThreadsX]; // per wave cluster indices
groupshared uint64_t nearestN[computeNumThreadsX];   // per wave nearest Neighbors

uint64_t encodeRelativeOffset(uint64_t id, uint64_t neighborID)
{
    var offset = neighborID - id;
    return abs(offset << 1) | (offset >> 31);
}

// Apetrei, 2014
bool findParentID(uint64_t L, uint64_t R, uint64_t clusterCount)
{
    return true; // TODO: impl
}

[require(compute, spirv_1_6)]
[numthreads(computeNumThreadsX, 1, 1)]
void buildCluster([[vk::push_constant]] BuildGeometryClusterPayload payload,
                  uint3 gid: SV_DispatchThreadID, uint3 groupThreadID: SV_GroupThreadID)
{
    uint localIdx = groupThreadID.x;
    uint globalIdx = gid.x;

    if (globalIdx >= payload.clusterCount)
        return;

    // HPLOC start
    var bvh2nodesBuffer = reinterpret<Ptr<BVH2NodeStorage>>(0); // BVH2 nodes, first entries are set to leaf bounding boxes
    var mortonCodes = reinterpret<Ptr<uint64_t>>(0);            // Morton codes sorted in ascending order
    var mcClusterIDs = reinterpret<Ptr<uint64_t>>(0);           // cluster indices extracted from sorted Morton codes
    var parentIDs = reinterpret<Ptr<uint64_t>>(0);              // BVH2parentIDs, initialized to −1

    // HPLOC algorithm (benthin et al, 2024)
    uint64_t L = globalIdx;
    uint64_t R = globalIdx;
    var laneActive = globalIdx < payload.clusterCount;

    [loop]
    while (laneActive) // do bottom-up traversal as long as active waves in wave
    {
        uint64_t split = 0;
        if (laneActive)
        {
            uint64_t previousID = 0;
            if (findParentID(L, R, payload.clusterCount))
            {
                InterlockedExchange(parentIDs[R], L, previousID);
                if (previousID != 1)
                {
                    split = R + 1;
                    R = previousID;
                }
            }
            else
            {
                InterlockedExchange(parentIDs[L - 1], R, previousID);
                if (previousID != 1)
                {
                    split = L;
                    L = previousID;
                }
            }
            if (previousID == -1)
                laneActive = false;
        }
        var size = R - L + 1;
        var final = laneActive & (size == payload.clusterCount); // reached top of morton tree
        var waveMask = WaveActiveBallot((laneActive & (size > (computeNumThreadsX / 2))) | final);
        while (any(waveMask > 0))
        {
            var laneID = firstbitlow(waveMask);
            
            // FUNCTION: plocMerge(laneID, L, R, split, final) // wave-based ploc++
            // var Lstart = WaveBroadcastLaneAt(L, laneID);
            // var Rend = WaveBroadcastLaneAt(R, laneID);
            // var Lend = WaveBroadcastLaneAt(split, laneID);
            // var Rstart = WaveBroadcastLaneAt(split, laneID);
            // var numLeft = 0;
            // var numRight = 0;
            // var numPrims = numLeft + numRight;
            // var threshold = WaveReadLaneFirst(final) == true ? 1 : computeNumThreadsX / 2;

            // while (numPrims > threshold)
            // {
                // function findNearestNeighbor() : benthin et al, 2022

                // end function
                // function mergeClustersCreateBVH2Nodes() : benthin et al, 2022

                // end function
            // }
            // store Indices

            // end function

            // waveMask = waveMask & (waveMask - 1); // done with current lane
        }
    }
}

[require(compute, spirv_1_6)]
[numthreads(computeNumThreadsX, 1, 1)]
void buildPartition([[vk::push_constant]] BuildScenePartitionPayload payload,
                    uint3 gid: SV_DispatchThreadID, uint3 groupThreadID: SV_GroupThreadID)
{
    uint localIdx = groupThreadID.x;
    uint globalIdx = gid.x;

    if (globalIdx >= payload.instanceCount)
        return;

    ScenePartitionBuildIndirectCommand cmd;
}

[require(compute, spirv_1_6)]
[numthreads(computeNumThreadsX, 1, 1)]
void updatePartition([[vk::push_constant]] UpdateScenePartitionPayload payload,
                     uint3 gid: SV_DispatchThreadID, uint3 groupThreadID: SV_GroupThreadID)
{
    uint localIdx = groupThreadID.x;
    uint globalIdx = gid.x;

    if (globalIdx >= payload.instanceCount)
        return;

    ScenePartitionUpdateIndirectCommand cmd;
}
