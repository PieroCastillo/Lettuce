/*
Created by @PieroCastillo on 2025-10-31
*/
struct VSOutput
{
    float4 position : SV_Position;
    float3 color : COLOR0;
};

[shader("vertex")]
VSOutput vertexMain(uint vertexID : SV_VulkanVertexID)
{
    VSOutput out;

    float2 positions[3] = {
        float2(0.0, -0.75),
        float2(0.75, 0.75),
        float2(-0.75, 0.75)
    };

    out.position = float4(positions[vertexID % 3], 0.0, 1.0);

    float3 colors[3] = {
        float3(1.0, 0.0, 0.0),
        float3(0.0, 1.0, 0.0),
        float3(0.0, 0.0, 1.0)
    };
    out.color = colors[vertexID % 3];

    return out;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target
{
    return float4(input.color, 1.0);
}