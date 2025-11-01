/*
Created by @PieroCastillo on 2025-10-31
*/
struct VertexInput
{
    float3 position : POSITION; // input del vertex buffer
};

struct VertexOutput
{
    float4 position : SV_Position; // posición en clip-space
    float4 color : COLOR;          // color pasado al fragment shader
};

StructuredBuffer<float4> uColor; // color uniforme

[shader("vertex")]
VertexOutput vertexMain(VertexInput vin)
{
    VertexOutput vout;
    vout.position = float4(vin.position, 1.0);
    vout.color = uColor[0];
    return vout;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput fin) : SV_Target
{
    return fin.color;
}
