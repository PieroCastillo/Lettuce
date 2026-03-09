/*
Created by @PieroCastillo on 2026-03-03
*/

// shader inputs
Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D storageTextures[];

struct VSOutput
{
    float4 position : SV_Position;
    float3 color : COLOR0;
    uint32_t instanceID;
};

struct SceneData
{
    float4x4 viewProj;
};

struct Instance
{
    uint32_t meshID;
    float4x4 model;
};

struct MeshInfo
{
    uint32_t primitiveOffset;
    uint32_t primitiveCount;
};

struct PrimitiveInfo
{
    uint32_t vertexOffset;
    uint32_t vertexCount;
    uint32_t indexOffset;
    uint32_t indexCount;
};

struct Vertex
{
    float3 position;
    float3 normal;
    float3 tangent;
    float2 texCoord0;
};

struct VkDrawIndirectCommand
{
    uint vertexCount;
    uint instanceCount;
    uint firstVertex;
    uint firstInstance;
};

struct InstanceBuffer
{
    uint32_t count;
    Instance instances[];
};

struct IndirectBuffer
{
    uint32_t count;
    VkDrawIndirectCommand cmds[];
};

struct InstancedPrimitive
{
    uint32_t primitiveID, instanceID;
};

struct PushConstants
{
    uint64_t ptrSceneData;
    uint64_t ptrInstances;
    uint64_t ptrMeshes;
    uint64_t ptrPrimitives;
    uint64_t ptrVertices;
    uint64_t ptrIndices;
    uint64_t ptrIndirectBuffer;
    uint64_t ptrInstancedPrimitives;
    uint64_t ptrPickInstanceData;
    uint64_t ptrDebugBuffer;
};

[[vk::push_constant]]
PushConstants pc;

[require(compute, spirv_1_6)]
[numthreads(32, 1, 1)]
void cullMain(uint3 id: SV_DispatchThreadID)
{
    uint instanceID = id.x;
    var indirectPtr = reinterpret<Ptr<IndirectBuffer>>(pc.ptrIndirectBuffer);

    if (instanceID >= reinterpret<Ptr<InstanceBuffer>>(pc.ptrInstances).count)
        return;

    var inst = reinterpret<Ptr<InstanceBuffer>>(pc.ptrInstances).instances[instanceID];
    var mesh = reinterpret<Ptr<MeshInfo>>(pc.ptrMeshes)[inst.meshID];
    var primitivesPtr = reinterpret<Ptr<PrimitiveInfo>>(pc.ptrPrimitives);
    var instancedPrimPtr = reinterpret<Ptr<InstancedPrimitive>>(pc.ptrInstancedPrimitives);

    uint drawIndex;
    InterlockedAdd(indirectPtr.count, mesh.primitiveCount, drawIndex);

    for (int i = 0; i < mesh.primitiveCount; i++)
    {
        var prim = primitivesPtr[mesh.primitiveOffset + i];

        var cmd = VkDrawIndirectCommand(prim.indexCount, 1, 0, 0);

        indirectPtr.cmds[drawIndex + i] = cmd;
        instancedPrimPtr[drawIndex + i] = InstancedPrimitive(mesh.primitiveOffset + i, instanceID);

        if (instanceID < 32)
        {
            reinterpret<Ptr<VkDrawIndirectCommand>>(pc.ptrDebugBuffer)[instanceID] = cmd; // for debugging
        }
    }
}

[shader("vertex")]
VSOutput vertexMain(uint vertexID: SV_VulkanVertexID, uint drawID: SV_DrawIndex)
{
    VSOutput out;

    var instancedPrim = reinterpret<Ptr<InstancedPrimitive>>(pc.ptrInstancedPrimitives)[drawID];
    var scene = reinterpret<Ptr<SceneData>>(pc.ptrSceneData)[0];
    var inst = reinterpret<Ptr<InstanceBuffer>>(pc.ptrInstances).instances[instancedPrim.instanceID];
    var prim = reinterpret<Ptr<PrimitiveInfo>>(pc.ptrPrimitives)[instancedPrim.primitiveID];
    var index = reinterpret<Ptr<uint32_t>>(pc.ptrIndices)[prim.indexOffset + vertexID];
    var v = reinterpret<Ptr<Vertex>>(pc.ptrVertices)[prim.vertexOffset + index];

    var worldPos = mul(inst.model, float4(v.position, 1.0));
    var clipPos = mul(scene.viewProj, worldPos);

    out.color = abs(v.normal);
    out.position = clipPos;
    out.instanceID = instancedPrim.instanceID;

    return out;
}

[require(fragment, spirv_1_6)]
[shader("fragment")]
float4 fragmentMain(VSOutput input, uint4 fragCoord: SV_Position) : SV_Target
{
    var pickTexture = storageTextures[0];
    var pickedInstance = reinterpret<Ptr<uint32_t>>(pc.ptrPickInstanceData)[0];
    var outColor = input.color;

    pickTexture[fragCoord.xy] = asfloat(input.instanceID + 1);

    if (pickedInstance == (input.instanceID + 1))
    {
        outColor.x = 1.0;
    }

    return float4(outColor, 1.0);
}
