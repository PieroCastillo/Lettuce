/*
Created by @PieroCastillo on 2025-02-14
*/
#include "structs.slang"

// shader inputs
Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D<float4> storageTextures[];

[require(compute, spirv)]
[shader("compute")]
void CompCullingPass([[vk::push_constant]] PCCullingPass pc)
{
    var sceneDataPtr = reinterpret<Ptr<SceneGPUData>>(pc.mvSceneAddress);
    var tlasNodesPtr = reinterpret<Ptr<TLASGPUItem>>(pc.mvTlasNodesAddress);
    var instancesPtr = reinterpret<Ptr<InstanceGPUItem>>(pc.mvInstancesAddress);
    var drawcallsPtr = reinterpret<Ptr<IndirectBuffer>>(pc.mvVisibilityPassDrawcallsAddress);
}

[require(amplification, spirv)]
[shader("amplification")]
[numthreads(32, 1, 1)]
void TaskVisibilityPass([[vk::push_constant]] PCVisibilityPass pc)
{
    var sceneDataPtr = reinterpret<Ptr<SceneGPUData>>(pc.mvSceneAddress);
    var instancesPtr = reinterpret<Ptr<InstanceGPUItem>>(pc.mvInstancesAddress);
    var blasNodesPtr = reinterpret<Ptr<BLASGPUItem>>(pc.mvBlasNodesAddress);
    var blasLeafMeshletIndicesPtr = reinterpret<Ptr<BlasLeafMeshletGPUItem>>(pc.mvBlasLeafMeshletIndicesAddress);
    var meshletsPtr = reinterpret<Ptr<MeshletGPUItem>>(pc.mvMeshletsAddress);
    var materialDataPtr = reinterpret<Ptr<MaterialDataGPUItem>>(pc.mvMaterialDatasAddress);
}

[require(mesh, spirv)]
[shader("mesh")]
[numthreads(32, 1, 1)]
MeshOutput MeshVisibilityPass(MeshVisibilityPassPayload payload, [[vk::push_constant]] PCVisibilityPass pc)
{
    var sceneDataPtr = reinterpret<Ptr<SceneGPUData>>(pc.mvSceneAddress);
    var instancesPtr = reinterpret<Ptr<InstanceGPUItem>>(pc.mvInstancesAddress);
    var blasNodesPtr = reinterpret<Ptr<BLASGPUItem>>(pc.mvBlasNodesAddress);
    var blasLeafMeshletIndicesPtr = reinterpret<Ptr<BlasLeafMeshletGPUItem>>(pc.mvBlasLeafMeshletIndicesAddress);
    var meshletsPtr = reinterpret<Ptr<MeshletGPUItem>>(pc.mvMeshletsAddress);
    var materialDataPtr = reinterpret<Ptr<MaterialDataGPUItem>>(pc.mvMaterialDatasAddress);
    // geometry buffers
    var indexBufferPtr = reinterpret<Ptr<uint32_t>>(pc.mvIndexBufferAddress);
    var vertexPosPtr = reinterpret<Ptr<float3>>(pc.mvVertexPosBufferAddress);
    var vertexNorPtr = reinterpret<Ptr<float3>>(pc.mvVertexNorBufferAddress);
    var vertexUVsPtr = reinterpret<Ptr<float2>>(pc.mvVertexUVsBufferAddress);

    MeshOutput output;

    return output;
}

[require(fragment, spirv)]
[shader("fragment")]
VisibilityBufferGPUPixel FragVisibilityPass(MeshOutput in) : SV_Target
{
    return in.data;
}

[require(compute, spirv)]
[shader("compute")]
[numthreads(16, 16, 1)]
void CompMaterialShadingPass([[vk::push_constant]] PCMaterialShadingPass pc, uint3 threadID : SV_DispatchThreadID)
{
    var sceneDataPtr = reinterpret<Ptr<SceneGPUData>>(pc.mvSceneAddress);
    var materialDataPtr = reinterpret<Ptr<MaterialDataGPUItem>>(pc.mvMaterialDatasAddress);
    // geometry buffers
    var indexBufferPtr = reinterpret<Ptr<uint32_t>>(pc.mvIndexBufferAddress);
    var vertexPosPtr = reinterpret<Ptr<float3>>(pc.mvVertexPosBufferAddress);
    var vertexNorPtr = reinterpret<Ptr<float3>>(pc.mvVertexNorBufferAddress);
    var vertexUVsPtr = reinterpret<Ptr<float2>>(pc.mvVertexUVsBufferAddress);

    var pixelID = threadID.xy;
    var visPixel = storageTextures[0][pixelID]; // tVisibilityTargetView
    var depthPixel = storageTextures[1][pixelID]; // tDepthTargetView
    var color = float4(1, 1, 1, 1);

    storageTextures[2][pixelID] = color; // tResultTargetView
}

// [require(vertex, spirv)]
// [shader("vertex")]
// void vertexMain(uint vertexID: SV_VulkanVertexID)
// {
//     VSOutput out;

//     float val = 0.5;
//     float2 positions[3] = {
//         float2(0.0, -val),
//         float2(val, val),
//         float2(-val, val)
//     };

//     Ptr<ParticleBuffer> particles = reinterpret<Ptr<ParticleBuffer>>(pc.particleAddress);
//     var particle = particles->particles[particles->idx];

//     out.position = float4(positions[vertexID % 3] + particle.pos, 0.0, 1.0);
//     out.color = particle.color;

//     return out;
// }

// [require(fragment, spirv)]
// [shader("fragment")]
// float4 fragmentMain(VSOutput input) : SV_Target
// {
//     return float4(input.color, 1.0);
// }