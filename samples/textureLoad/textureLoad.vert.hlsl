struct VSOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    uint instanceID : TEXCOORD1;
};

VSOutput main(uint vertexID: SV_VertexID, uint instanceID: SV_InstanceID)
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