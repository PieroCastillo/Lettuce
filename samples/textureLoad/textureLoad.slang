Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D<float4> storageTextures[];

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv;
    uint instanceID;
};

[require(vertex, spirv)]
[shader("vertex")]
VSOutput vertMain(uint vertexID: SV_VulkanVertexID, uint instanceID: SV_VulkanInstanceID)
{
    VSOutput output;
    float2 positions[6] = {
        float2(0.0, 0.0), // top-left
        float2(0.0, 1.0), // bottom-left
        float2(1.0, 0.0), // top-right
        float2(1.0, 0.0), // top-right
        float2(0.0, 1.0), // bottom-left
        float2(1.0, 1.0)  // bottom-right
    };

    float2 uvs[6] = {
        float2(0.0, 1.0),
        float2(0.0, 0.0),
        float2(1.0, 1.0),
        float2(1.0, 1.0),
        float2(0.0, 0.0),
        float2(1.0, 0.0)
    };

    float2 gridPos = float2(instanceID % 2, instanceID / 2);
    float2 localPos = positions[vertexID];
    float2 finalPos = (localPos * 0.5) + (gridPos * 0.5);

    output.position = float4(finalPos.x * 2.0 - 1.0, -(finalPos.y * 2.0 - 1.0), 0.0, 1.0);
    output.uv = uvs[vertexID];
    output.instanceID = instanceID;

    return output;
}

[require(fragment, spirv_1_6)]
[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target
{
    return sampledTextures[input.instanceID].Sample(samplers[0], saturate(input.uv));
}