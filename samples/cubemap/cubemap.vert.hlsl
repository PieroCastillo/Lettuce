struct VSOut {
    float4 pos : SV_Position;
    float3 dir : TEXCOORD0;
};

VSOut main(uint vid : SV_VertexID)
{
    float2 pos[3] = {
        float2(-1, -1),
        float2(-1,  3),
        float2( 3, -1)
    };

    VSOut o;
    o.pos = float4(pos[vid], 0.0, 1.0);

    // Dirección de cubemap (espacio de vista)
    float2 uv = pos[vid];
    o.dir = normalize(float3(uv.x, -uv.y, 1.0));

    return o;
}