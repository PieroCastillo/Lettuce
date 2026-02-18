/*
Created by @PieroCastillo on 2025-02-14
*/
#include "structs.slang"

// shader inputs
Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D<float4> storageTextures[];

groupshared uint taskShaderGroupSizeX;

[require(compute, spirv_1_4)]
[shader("compute")]
void CompCullingPass([[vk::push_constant]] uniform PCCullingPass pc, uint3 threadID: SV_DispatchThreadID)
{
    var sceneDataPtr = reinterpret<Ptr<SceneGPUData>>(pc.mvSceneAddress);
    var tlasNodesPtr = reinterpret<Ptr<BVHNodeGPUItem>>(pc.mvTlasNodesAddress);
    var instancesPtr = reinterpret<Ptr<InstanceGPUItem>>(pc.mvInstancesAddress);
    var drawcallsPtr = reinterpret<Ptr<IndirectBuffer>>(pc.mvVisibilityPassDrawcallsAddress);
    var taskRecordPtr = reinterpret<Ptr<TaskRecord>>(pc.mvTaskRecordsAddress);

    var idx = threadID.x;
    if (idx == 0)
    {
        taskShaderGroupSizeX = 0;
    }
    GroupMemoryBarrierWithGroupSync();

    var instance = instancesPtr[idx];

    // TODO: apply culling per instance, if pass frustrum culling test
    if (true)
    {
        taskRecordPtr[idx].instanceID = idx;
        var groupsPerInstance = (uint32_t)ceil(instance.meshletCount / 64.0f);
        InterlockedAdd(taskShaderGroupSizeX, groupsPerInstance);
    }

    if (idx == 0)
    {
        var drawcall = VkDrawMeshTasksIndirectCommandEXT(taskShaderGroupSizeX, 0, 0);
        drawcallsPtr->count = 1;
        drawcallsPtr->cmds[0] = drawcall;
    }
}

[require(amplification, spirv)]
[shader("amplification")]
[numthreads(computeNumthreadsSizeX, 1, 1)]
void TaskVisibilityPass([[vk::push_constant]] uniform PCVisibilityPass pc, uint3 threadID: SV_DispatchThreadID)
{
    var sceneDataPtr = reinterpret<Ptr<SceneGPUData>>(pc.mvSceneAddress);
    var taskRecordsPtr = reinterpret<Ptr<TaskRecord>>(pc.mvTaskRecordsAddress);
    var instancesPtr = reinterpret<Ptr<InstanceGPUItem>>(pc.mvInstancesAddress);
    var blasNodesPtr = reinterpret<Ptr<BVHNodeGPUItem>>(pc.mvBlasNodesAddress);
    var meshletsPtr = reinterpret<Ptr<MeshletGPUItem>>(pc.mvMeshletsAddress);

    var instance = instancesPtr[taskRecordsPtr[threadID.x].instanceID];

    MeshVisibilityPassPayload payload;

    // TODO:apply culling per meshlet

    DispatchMesh(1, 0, 0, payload); 
}

groupshared float4x4 projViewModel;
groupshared MeshletGPUItem meshletInfo;

[require(mesh, spirv)]
[shader("mesh")]
[numthreads(computeNumthreadsSizeX, 1, 1)]
void MeshVisibilityPass(out indices uint3 triangles[meshletTriangleCount], out vertices MeshVertexOutput vertices[meshletVertexCount],
                        in MeshVisibilityPassPayload payload, [[vk::push_constant]] uniform PCVisibilityPass pc,
                        uint3 threadID: SV_DispatchThreadID)
{
    var idx = threadID.x;

    if (idx == 0)
    {
        // calculate pvm matrix once
        var sceneDataPtr = reinterpret<Ptr<SceneGPUData>>(pc.mvSceneAddress);
        var instancesPtr = reinterpret<Ptr<InstanceGPUItem>>(pc.mvInstancesAddress);
        projViewModel = sceneDataPtr->projection * sceneDataPtr->view * instancesPtr[payload.instanceID].model;

        // get meshlet info once
        var meshletsPtr = reinterpret<Ptr<MeshletGPUItem>>(pc.mvMeshletsAddress);
        meshletInfo = meshletsPtr[payload.meshletID];
    }
    GroupMemoryBarrierWithGroupSync();

    var indexBufferPtr = reinterpret<Ptr<uint32_t>>(pc.mvIndexBufferAddress);
    var vertexPosPtr = reinterpret<Ptr<float3>>(pc.mvVertexPosBufferAddress);

    SetMeshOutputCounts(meshletTriangleCount, meshletVertexCount);

    var vertexCount = meshletInfo.vertexCount;
    var triangleCount = meshletInfo.triangleCount;
    
    for (int i = idx; i < vertexCount; i += computeNumthreadsSizeX)
    {
        vertices[i].pos = float4(vertexPosPtr[meshletInfo.vertexOffset + i], 1);
        vertices[i].data.instanceID = payload.instanceID;
        vertices[i].data.meshletID = payload.meshletID;
        vertices[i].data.LOD = 0; // TODO: calculate LOD
        vertices[i].data.triangleID = 0; // TODO: calculate triangle ID
    }

    for (int i = idx; i < triangleCount; i += computeNumthreadsSizeX)
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