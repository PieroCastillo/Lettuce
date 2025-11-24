
/* Copyright (c) 2025, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

struct VertexOutput
{
    float4 position : SV_Position;
    float4 color;
};

static float4 positions[3] = {
    float4(0.0, -1.0, 0.0, 1.0),
    float4(-1.0, 1.0, 0.0, 1.0),
    float4(1.0, 1.0, 0.0, 1.0)
};

static float4 colors[3] = {
    float4(0.0, 1.0, 0.0, 1.0),
    float4(0.0, 0.0, 1.0, 1.0),
    float4(1.0, 0.0, 0.0, 1.0)
};

struct DummyPayLoad
{
    uint dummyData;
};

// We don't use pay loads in this sample, but the fn call requires one
groupshared DummyPayLoad dummyPayLoad;

[shader("amplification")]
[numthreads(1, 1, 1)]
void amplificationMain()
{
    DispatchMesh(3, 1, 1, dummyPayLoad);
}

[shader("mesh")]
[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void meshMain(out indices uint3 triangles[1], out vertices VertexOutput vertices[3], in payload DummyPayLoad ply, uint3 DispatchThreadID: SV_DispatchThreadID)
{
    SetMeshOutputCounts(3, 1);

    float4 offset = float4(0.0, 0.0, float(DispatchThreadID.x), 0.0);

    for (uint i = 0; i < 3; i++) {
        vertices[i].position = 0.5*(positions[i] + offset);
        vertices[i].color = colors[i];
    }

    triangles[0] = uint3(0, 1, 2);
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    return input.color;
}