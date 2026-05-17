Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D<float4> storageImages[];

struct PushConstants
{
    float3 *uniformAddress;
};
[[vk::push_constant]]
PushConstants pc;

struct VSOutput
{
    float4 position : SV_Position;
    float3 color : COLOR0;
};

[shader("vertex")]
VSOutput vertexMain(uint vertexID: SV_VertexID)
{
    VSOutput out;

    float2 positions[4] = {
        float2(-0.75, -0.75),
        float2(0.75, -0.75),
        float2(0.75, 0.75),
        float2(-0.75, 0.75),
    };

    float3 colors[4] = {
        float3(1, 0, 0),
        float3(0, 1, 0),
        float3(0, 0, 1),
        float3(0, 1, 0),
    };

    uint indices[6] = {
        0, 1, 2,
        0, 2, 3
    };

    uint idx = indices[vertexID];

    out.position = float4(positions[idx], 0.0, 1.0);
    out.color = *pc.uniformAddress;
    return out;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target
{
    return float4(input.color, 1.0);
}