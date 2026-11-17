module LettuceRendering;

__include "../shaderShared/std.part.slang";
__include "../shaderShared/std.shapes.part.slang";
__include "../shaderShared/std.shapes.gen.part.slang";
__include "../shaderShared/std.procedural.part.slang";

Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D<float4> storageTextures[];

struct PushConstants
{
    uint64_t sceneViewDataAddr;
    uint64_t materialShadingDataAddr;
    uint64_t materialStorageAddr;
    uint64_t proceduralInstanceStorageAddr;
    uint64_t indirectCommandsStorageAddr;

    uint64_t sphereStorageAddr;
    uint64_t prismStorageAddr;
    uint64_t cylinderStorageAddr;
};

[[vk::push_constant]]
PushConstants pc;

[shader("compute")]
[numthreads(32, 1, 1)]
void generateDrawCalls(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    var scene = std.loadAddr<std.SceneViewData>(pc.sceneViewDataAddr);
    var instancesData = std.loadAddr<std.ProceduralInstanceData>(pc.proceduralInstanceStorageAddr);
    var indirectData = std.loadAddr<std.IndirectCommandStorage>(pc.indirectCommandsStorageAddr);

    // only one thread writes the draw count
    if (index == 0)
        indirectData->count = min(scene->instanceCount, instancesData->count);

    uint instanceCount = min(scene->instanceCount, instancesData->count);

    if (index >= instanceCount)
        return;

    GroupMemoryBarrierWithGroupSync();

    var instance = instancesData->instances[index];
    uint vertexCount = 0;

    switch (instance.geometryType)
    {
    case std.ProceduralShapeGeometryType.Sphere:
    {
        var storage = std.loadAddr<std.SphereStorage>(pc.sphereStorageAddr);

        if (instance.geometryIdx >= instancesData->count)
            return;

        vertexCount = std.getSphereVertexCount(storage[instance.geometryIdx]);
        break;
    }
    case std.ProceduralShapeGeometryType.Prism:
    {
        var storage = std.loadAddr<std.PrismStorage>(pc.prismStorageAddr);

        if (instance.geometryIdx >= instancesData->count)
            return;

        vertexCount = std.getPrismVertexCount(storage[instance.geometryIdx]);
        break;
    }
    case std.ProceduralShapeGeometryType.Cylinder:
    {
        var storage = std.loadAddr<std.CylinderStorage>(pc.cylinderStorageAddr);

        if (instance.geometryIdx >= instancesData->count)
            return;

        vertexCount = std.getCylinderVertexCount(storage[instance.geometryIdx]);
        break;
    }
    default:
        return;
    }

    // One indirect draw per procedural instance.
    std.VkDrawIndirectCommand command;

    command.vertexCount = vertexCount;
    command.instanceCount = 1;
    command.firstVertex = 0;
    command.firstInstance = 0;

    indirectData.commands[index] = command;
}

[shader("vertex")]
std.DeferredVSOutput vertMain(uint vertexID: SV_VulkanVertexID, uint instanceID: SV_DrawIndex)
{
    var sceneViewData = std.loadAddr<std.SceneViewData>(pc.sceneViewDataAddr);
    var instancesData = std.loadAddr<std.ProceduralInstanceData>(pc.proceduralInstanceStorageAddr);
    var indirectData = std.loadAddr<std.IndirectCommandStorage>(pc.indirectCommandsStorageAddr);
    var spheres = std.loadAddr<std.SphereStorage>(pc.sphereStorageAddr);
    var prisms = std.loadAddr<std.PrismStorage>(pc.prismStorageAddr);
    var cylinders = std.loadAddr<std.CylinderStorage>(pc.cylinderStorageAddr);
    var instance = instancesData->instances[instanceID];

    std.DeferredVSOutput output;
    var localPosition = float3(0, 0, 0);
    var localNormal = float3(0, 1, 0);
    var localTangent = float3(1, 0, 0);
    var uv = float2(0, 0);

    switch (instance.geometryType)
    {
    case std.ProceduralShapeGeometryType.Sphere:
    {
        std.generateSphereVertex(spheres[instance.geometryIdx], vertexID, localPosition, localNormal, localTangent, uv);
        break;
    }

    case std.ProceduralShapeGeometryType.Prism:
    {
        std.generatePrismVertex(prisms[instance.geometryIdx], vertexID, localPosition, localNormal, localTangent, uv);
        break;
    }

    case std.ProceduralShapeGeometryType.Cylinder:
    {
        std.generateCylinderVertex(cylinders[instance.geometryIdx], vertexID, localPosition, localNormal, localTangent, uv);
        break;
    }

    default:
    {
        output.position = float4(0, 0, 0, 1);
        output.uv = float2(0, 0);
        output.worldNormal = float3(0, 1, 0);
        output.worldTangent = float4(1, 0, 0, 1);
        output.materialID = 0;
        output.pickID = 0;
        return output;
    }
    }

    // local -> world
    var worldPosition = mul(instance.transform, float4(localPosition, 1.0));
    var normalMatrix = transpose((float3x3)instance.invTransform);
    var worldNormal = normalize(mul(normalMatrix, localNormal));
    var worldTangent = normalize(mul((float3x3)instance.transform, localTangent));

    // gram-schmidt orthogonalization
    worldTangent = normalize(worldTangent - worldNormal * dot(worldNormal, worldTangent));

    // world -> clip
    output.position = mul(sceneViewData->viewProj, worldPosition);
    output.uv = uv;
    output.worldNormal = worldNormal;
    output.worldTangent = float4(worldTangent, 1.0);
    output.materialID = instance.materialIdx;
    output.pickID = instanceID + 1; // reserve zero for background

    return output;
}

[shader("fragment")]
std.DeferredPSOutput fragMain(std.DeferredVSOutput input)
{
    return std.deferredFragBase(input);
}