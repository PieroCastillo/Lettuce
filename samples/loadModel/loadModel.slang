/*
Created by @PieroCastillo on 2026-03-03
*/
struct VSOutput
{
    float4 position : SV_Position;
    float3 color : COLOR0;
};

struct SceneData
{
    float4x4 viewProjection;
};

struct PushConstants
{
    uint64_t ptrSceneData;
};

[[vk::push_constant]]
PushConstants pc;

[require(vertex, spirv_latest)]
[shader("vertex")]
VSOutput vertexMain(uint vertexID: SV_VulkanVertexID)
{
    VSOutput out;

    float2 positions[3] = {
        float2(0.0, -0.75),
        float2(0.75, 0.75),
        float2(-0.75, 0.75)
    };

    out.position = 150 * float4(positions[vertexID % 3], 0.0, 1.0);

    float3 colors[3] = {
        float3(1.0, 0.0, 0.0),
        float3(0.0, 1.0, 0.0),
        float3(0.0, 0.0, 1.0)
    };
    out.color = colors[vertexID % 3];

    var vp = reinterpret<Ptr<SceneData>>(pc.ptrSceneData)->viewProjection;
    out.position = mul(out.position, vp);

    return out;
}

[require(fragment, spirv)]
[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target
{
    return float4(input.color, 1.0);
}
