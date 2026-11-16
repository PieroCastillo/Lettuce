module LettuceRendering;

__include "../shaderShared/std.part.slang";

const static uint maxTriangles = 126;
const static uint maxIndices = maxTriangles * 3;

Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D<float4> storageTextures[];

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv;
};

struct PSOutput
{
    float4 colorOutput : SV_Target0;
    uint32_t pickOutput : SV_Target1;
};

struct PushConstants
{
    uint64_t sceneViewDataAddr;
    uint64_t materialShadingDataAddr;
    uint64_t materialStorageAddr;
    uint64_t sunLightAddr;
    uint64_t spotLightStorageAddr;
    uint64_t csmDataAddr;
};

[[vk::push_constant]]
PushConstants pc;

[shader("vertex")]
VSOutput vertMain(uint vertexID: SV_VulkanVertexID, uint instanceID: SV_DrawIndex)
{
    VSOutput output;
    var uv = float2((vertexID << 1) & 2, vertexID & 2);
    output.position = float4(uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);

    return output;
}

/*
expected target:
R:           u     |       v
G:    ddx length   |   ddy length
B: ddx rot|ddy rot |   materialID
A:         tangent space
*/
[require(fragment, spirv_1_6)]
[shader("fragment")]
PSOutput fragMain(VSOutput input)
{
    var sceneViewData = std.loadAddr<std.SceneViewData>(pc.sceneViewDataAddr)[0];
    var passData = std.loadAddr<std.MaterialShadingData>(pc.materialShadingDataAddr)[0];
    var materialsAddr = std.loadAddr<std.MaterialPBRStorage>(pc.materialStorageAddr);
    var sun = std.loadAddr<std.lighting.SunLightData>(pc.sunLightAddr)[0];
    var spots = std.loadAddr<std.lighting.SpotLightData>(pc.spotLightStorageAddr);
    var csm = std.loadAddr<std.lighting.CascadeShadowMapData>(pc.csmDataAddr)[0];
    var pixelCoord = uint3(uint2(input.position.xy), 0);
    var pi = radians(180);

    var depthTexture = sampledTextures[0]; // passData.depthTextureIdx];
    var gbuffer = sampledTextures[passData.gbufferTextureIdx];
    var raw = asuint(gbuffer.Load(pixelCoord));

    // gbuffer decompression
    // uv
    var uv = std.unpackUnorm2x16(raw.r);

    // ddx, ddy
    var lenDdx = f16tof32(raw.g & 0xFFFF);
    var lenDdy = f16tof32((raw.g >> 16) & 0xFFFF);
    var normRotX = float(raw.b & 0xFF) / 255.0;
    var normRotY = float((raw.b >> 8) & 0xFF) / 255.0;
    // material
    uint materialID = (raw.b >> 16) & 0xFFFF;

    var angleX = normRotX * (2 * pi) - pi;
    var angleY = normRotY * (2 * pi) - pi;

    var uvDdx = float2(cos(angleX), sin(angleX)) * lenDdx;
    var uvDdy = float2(cos(angleY), sin(angleY)) * lenDdy;

    var q = std.unpackUnorm4x8(raw.a) * 2.0 - 1.0;
    q = normalize(q); // correction for 8-bit quantization
    var TBN = std.quaternionToTBN(q);
    var material = materialsAddr[materialID];

    // surface data
    float depth = depthTexture.Load(pixelCoord).r;

    var worldH = mul(sceneViewData.invViewProj, float4(uv, depth, 1.0));
    var worldPos = worldH.xyz * rcp(worldH.w);
    var N = normalize(TBN[2]);
    var V = normalize(sceneViewData.cameraPosition - worldPos);

    float viewDepth = dot(worldPos - sceneViewData.cameraPosition, sceneViewData.cameraForward);

    var color = material.emissive;                                                                             // PBR
    var shadow = std.lighting.sampleCSM(sampledTextures, csm, worldPos, N, normalize(sun.direction), viewDepth); // Sun + CSM
    color += std.lighting.evaluateSun(sun, N, V, material.baseColor.rgb, material.metallic, material.roughness, shadow);

    // spot lights
    for (uint i = 0; i < min(spots.count, std.lighting.MaxSpotLights); ++i)
    {
        color += std.lighting.evaluateSpot(spots, i, worldPos, N, V, material.baseColor.rgb, material.metallic, material.roughness);
    }
    PSOutput output;
    output.colorOutput = float4(color, material.baseColor.a);
    return output;
}