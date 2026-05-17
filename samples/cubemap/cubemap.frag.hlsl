Texture2D<float4> sampledTextures[] : register(t0, space0);
SamplerState samplers[] : register(s1, space0);

struct VSOut {
    float4 pos : SV_Position;
    float3 dir : TEXCOORD0;
};

float4 main(VSOut input) : SV_Target
{
    // uint idx = NonUniformResourceIndex(input.instanceID);
    return float4(0.5f, 0.5f, 0.5f, 1.0f);// sampledTextures[idx].Sample(samplers[0], saturate(input.uv));
}