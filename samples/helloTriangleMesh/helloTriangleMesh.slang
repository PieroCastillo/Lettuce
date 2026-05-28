struct DummyPayLoad
{
    uint dummyData;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float4 color : COLOR0;
};

// We don't use pay loads in this sample, but the fn call requires one
groupshared DummyPayLoad dummyPayLoad;

[shader("amplification")]
[numthreads(1,1,1)]
void taskMain(uint3 groupThreadID: SV_GroupThreadID, uint3 dispatchThreadID: SV_DispatchThreadID)
{
    DispatchMesh(3, 1, 1, dummyPayLoad);
}

[require(mesh, spirv_1_6)]
[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void meshMain(out indices uint3 triangles[1], out vertices VertexOutput vertices[3], uint3 DispatchThreadID: SV_DispatchThreadID)
{
    SetMeshOutputCounts(3, 1);

    float4 positions[3] = {
        float4(-0.75, 0.75, 0.0, 1.0),
        float4(0.75, 0.75, 0.0, 1.0),
        float4(0.0, -0.75, 0.0, 1.0),
    };

    float4 colors[3] = {
        float4(1.0, 0.0, 0.0, 1.0),
        float4(0.0, 1.0, 0.0, 1.0),
        float4(0.0, 0.0, 1.0, 1.0),
    };

    for (uint i = 0; i < 3; i++)
    {
        vertices[i].position = positions[i];
        vertices[i].color = colors[i];
    }

    triangles[0] = uint3(0, 1, 2);
}

[shader("fragment")]
float4 fragMain(VertexOutput input) : SV_TARGET
{
    return float4(input.color);
}