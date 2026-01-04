/*
Created by @PieroCastillo on 2025-11-15
*/
struct VSInput
{
    float3 position : POSITION;
    float3 color : COLOR0;
};

struct CubeInstanceInfo
{
    float3 position;
};

struct WorldInfo
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
};

struct ShaderInput
{
    ConstantBuffer<WorldInfo> worldInfo;
    ConstantBuffer<CubeInstanceInfo> cubeInstances[];
};
ParameterBlock<ShaderInput> shaderInput;

struct VSOutput
{
    float3 position : SV_Position;
    float3 color : COLOR0;
};

[shader("vertex")]
VSOutput vertexMain(VSInput input, uint instanceID : SV_VulkanInstanceID)
{
    VSOutput out;

    float4x4 p = shaderInput.worldInfo.projection;
    float4x4 v = shaderInput.worldInfo.view;
    float4x4 m = shaderInput.worldInfo.model;
    float4x4 res = mul(p, mul(v, m));

    out.position =  mul(res, float4(input.position + shaderInput.cubeInstances[instanceID].position, 1.0)).xyz;
    out.color = input.color;

    return out;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target
{
    return float4(input.color, 1.0);
}