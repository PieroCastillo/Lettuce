/*
Created by @PieroCastillo on 2025-02-14
*/
#include "structs.slang"

// shader inputs
Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D<float4> storageTextures[];

groupshared uint taskShaderGroupSizeX;
groupshared uint meshletsLeft;
groupshared InstanceGPUItem instanceInfo;

/*
Compute shader traverses TLAS
*/
[require(compute, spirv_1_4)]
[shader("compute")]
[numthreads(computeNumthreadsSizeX, 1, 1)]
void CompCullingPass([[vk::push_constant]] uniform PCCullingPass pc,
                     uint3 threadID: SV_DispatchThreadID, uint3 localID: SV_GroupThreadID)
{
    // load initial data
    var sceneDataPtr = reinterpret<Ptr<SceneGPUData>>(pc.mvSceneAddress);
    var tlasNodesPtr = reinterpret<Ptr<BVHNodeGPUItem>>(pc.mvTlasNodesAddress);
    var instancesPtr = reinterpret<Ptr<InstanceGPUItem>>(pc.mvInstancesAddress);
    var drawcallsPtr = reinterpret<Ptr<IndirectBuffer>>(pc.mvVisibilityPassDrawcallsAddress);
    var taskRecordPtr = reinterpret<Ptr<TaskRecord>>(pc.mvTaskRecordsAddress);

    var lidx = localID.x;
    var gidx = threadID.x;

    if(gidx == 0)
        drawcallsPtr->count = 1;
    
    if (lidx == 0)
    {
        taskShaderGroupSizeX = 0;
        instanceInfo = instancesPtr[gidx];
        meshletsLeft = instanceInfo.meshletCount;
    }
    GroupMemoryBarrierWithGroupSync();

    // TODO: apply culling per instance, if pass frustrum culling test
    if (true)
    {
        // taskRecordPtr[idx].instanceID = idx;
        // var groupsPerInstance = (uint32_t)ceil(instance.meshletCount / maxMeshletPerTaskGroup);
        // InterlockedAdd(taskShaderGroupSizeX, groupsPerInstance);
    }

    GroupMemoryBarrierWithGroupSync();
    if (lidx == 0) // one thread writes drawcalls only, one drawcall at total
    {
        InterlockedAdd(drawcallsPtr->cmds[0].groupCountX, taskShaderGroupSizeX);
    }
}

groupshared TaskRecord taskRecord;
groupshared uint meshletCounter;
groupshared MeshVisibilityPassPayload meshPayload;

/*
Task shader traverses BLAS, each task group cooperates to traverse a BLAS subtree
*/
[require(amplification, spirv)]
[shader("amplification")]
[numthreads(computeNumthreadsSizeX, 1, 1)]
void TaskVisibilityPass([[vk::push_constant]] uniform PCVisibilityPass pc,
                        uint3 threadID: SV_DispatchThreadID, uint3 localID: SV_GroupThreadID, uint3 groupID: SV_GroupID)
{
    // load initial data
    var sceneDataPtr = reinterpret<Ptr<SceneGPUData>>(pc.mvSceneAddress);
    var taskRecordsPtr = reinterpret<Ptr<TaskRecord>>(pc.mvTaskRecordsAddress);
    var instancesPtr = reinterpret<Ptr<InstanceGPUItem>>(pc.mvInstancesAddress);
    var blasNodesPtr = reinterpret<Ptr<BVHNodeGPUItem>>(pc.mvBlasNodesAddress);
    var meshletsPtr = reinterpret<Ptr<MeshletGPUItem>>(pc.mvMeshletsAddress);
    var gidx = groupID.x;

    if (gidx == 0)
    {
        taskRecord = taskRecordsPtr[gidx]; // selects task record by group ID
        meshletCounter = 0;                // init var
        meshPayload.instanceID = taskRecord.instanceID;
    }
    GroupMemoryBarrierWithGroupSync();

    // var instance = instancesPtr[taskRecordsPtr[threadID.x].instanceID]; // how to access instance data

    // TODO: apply culling per meshlet

    GroupMemoryBarrierWithGroupSync();
    if (gidx == 0)
        DispatchMesh(meshletCounter, 1, 1, meshPayload);
}

groupshared float4x4 projViewModel;
groupshared MeshletGPUItem meshletInfo;

[require(mesh, spirv)]
[shader("mesh")]
[numthreads(computeNumthreadsSizeX, 1, 1)]
void MeshVisibilityPass(out indices uint3 triangles[meshletTriangleCount], out vertices MeshVertexOutput vertices[meshletVertexCount],
                        in MeshVisibilityPassPayload payload, [[vk::push_constant]] uniform PCVisibilityPass pc,
                        uint3 threadID: SV_DispatchThreadID, uint3 localID: SV_GroupThreadID, uint3 groupID: SV_GroupID)
{
    var idx = threadID.x;
    var lidx = localID.x;
    var gidx = groupID.x;

    if (idx == 0)
    {
        // calculate pvm matrix once
        var sceneDataPtr = reinterpret<Ptr<SceneGPUData>>(pc.mvSceneAddress);
        var instancesPtr = reinterpret<Ptr<InstanceGPUItem>>(pc.mvInstancesAddress);
        projViewModel = sceneDataPtr->projection * sceneDataPtr->view * instancesPtr[payload.instanceID].model;

        // get meshlet info once
        var meshletsPtr = reinterpret<Ptr<MeshletGPUItem>>(pc.mvMeshletsAddress);
        meshletInfo = meshletsPtr[payload.meshletIDs[gidx]];
    }
    GroupMemoryBarrierWithGroupSync();

    var indexBufferPtr = reinterpret<Ptr<uint32_t>>(pc.mvIndexBufferAddress);
    var vertexPosPtr = reinterpret<Ptr<float3>>(pc.mvVertexPosBufferAddress);

    SetMeshOutputCounts(meshletTriangleCount, meshletVertexCount);

    var vertexCount = meshletInfo.vertexCount;
    var triangleCount = meshletInfo.triangleCount;

    // vertex/index selection depends in local thread ID
    for (int i = lidx; i < vertexCount; i += computeNumthreadsSizeX)
    {
        vertices[i].pos = float4(vertexPosPtr[meshletInfo.vertexOffset + i], 1);
        vertices[i].data.instanceID = payload.instanceID;
        vertices[i].data.meshletID = payload.meshletIDs[gidx];
        vertices[i].data.LOD = 0;        // TODO: calculate LOD
        vertices[i].data.triangleID = 0; // TODO: calculate triangle ID
    }

    for (int i = lidx; i < triangleCount; i += computeNumthreadsSizeX)
    {
        triangles[i] = indexBufferPtr[meshletInfo.triangleOffset + i];
    }
}

[require(fragment, spirv)]
[shader("fragment")]
VisibilityBufferGPUPixel FragVisibilityPass(MeshVertexOutput in) : SV_Target
{
    return in.data;
}

[require(compute, spirv)]
[shader("compute")]
[numthreads(tileSizeX, tileSizeY, 1)]
void CompMaterialShadingPass([[vk::push_constant]] uniform PCMaterialShadingPass pc, uint3 threadID: SV_DispatchThreadID)
{
    var sceneDataPtr = reinterpret<Ptr<SceneGPUData>>(pc.mvSceneAddress);
    var materialDataPtr = reinterpret<Ptr<MaterialDataGPUItem>>(pc.mvMaterialDatasAddress);
    // geometry buffers
    var indexBufferPtr = reinterpret<Ptr<uint32_t>>(pc.mvIndexBufferAddress);
    var vertexPosPtr = reinterpret<Ptr<float3>>(pc.mvVertexPosBufferAddress);
    var vertexNorPtr = reinterpret<Ptr<float3>>(pc.mvVertexNorBufferAddress);
    var vertexUVsPtr = reinterpret<Ptr<float2>>(pc.mvVertexUVsBufferAddress);

    var pixelID = threadID.xy;
    var visPixel = storageTextures[0][pixelID];   // tVisibilityTargetView
    var depthPixel = storageTextures[1][pixelID]; // tDepthTargetView
    var color = float4(1, 1, 1, 1);

    // here we need to resolve color

    storageTextures[2][pixelID] = color; // tResultTargetView
}