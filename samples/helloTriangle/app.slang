/*
Created by @PieroCastillo on 2025-10-31
*/
struct VertexInput
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float4 tangent  : TANGENT;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
};

struct ShaderParams
{
    float4 uColor;
    float4x4 model;
    float4x4 view;
    float4x4 projection;
}

ParameterBlock<ShaderParams> params;

[shader("vertex")]
VertexOutput vertexMain(VertexInput vin)
{
    VertexOutput vout;
    vout.position = mul((params.projection * params.view * params.model), float4(vin.position, 1.0));
    vout.normal = vin.normal;
    vout.tangent = vin.tangent;
    return vout;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput fin) : SV_Target
{
    return params.uColor;
}