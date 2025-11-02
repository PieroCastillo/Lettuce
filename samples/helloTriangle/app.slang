/*
Created by @PieroCastillo on 2025-10-31
*/
struct VertexInput
{
    float3 position;
};

struct VertexOutput
{
    float4 position : SV_Position;
};

struct ShaderParams
{
    float4 uColor;
}

ParameterBlock<ShaderParams> params;

[shader("vertex")]
VertexOutput vertexMain(VertexInput vin)
{
    VertexOutput vout;
    vout.position = float4(vin.position, 1.0);
    return vout;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput fin) : SV_Target
{
    return params.uColor;
}