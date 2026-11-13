module LettuceRendering;

__include "../shaderShared/types.part.slang";
__include "../shaderShared/materials.part.slang";
__include "../shaderShared/funcs.part.slang";

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
    uint64_t materialStorageAddr;
};

Ptr<T> loadAddr<T>(uint64_t addr, uint32_t offset = 0)
{
    return reinterpret<Ptr<T>>(addr + (offset * sizeof(T)));
}

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
    var sceneViewData = loadAddr<SceneViewData>(pc.sceneViewDataAddr)[0];
    var materialsAddr = loadAddr<MaterialPBRStorage>(pc.materialStorageAddr);
    constexpr var pi = radians(180);

    float4 buffer;
    var raw = asuint(buffer);

    var uv = unpackUnorm2x16(raw.r);

    var lenDdx = f16tof32(raw.g & 0xFFFF);
    var lenDdy = f16tof32((raw.g >> 16) & 0xFFFF);

    var normRotX = float(raw.b & 0xFF) / 255.0;
    var normRotY = float((raw.b >> 8) & 0xFF) / 255.0;
    uint materialID = (raw.b >> 16) & 0xFFFF;

    var angleX = normRotX * (2 * pi) - pi;
    var angleY = normRotY * (2 * pi) - pi;

    var uvDdx = float2(cos(angleX), sin(angleX)) * lenDdx;
    var uvDdy = float2(cos(angleY), sin(angleY)) * lenDdy;

    var q = unpackUnorm4x8(raw.a) * 2.0 - 1.0;
    q = normalize(q); // correction for  8-bit quantization
    var TBN = quaternionToTBN(q);
    var material = materialsAddr[materialID];

    // here: resolve material & lighting

    PSOutput output;
    output.colorOutput = float4(1);

    return output;
}