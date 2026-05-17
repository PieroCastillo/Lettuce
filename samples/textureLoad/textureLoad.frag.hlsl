Texture2D<float4> sampledTextures[] : register(t0, space0);
SamplerState samplers[] : register(s1, space0);

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    uint instanceID : TEXCOORD1;
};

float4 main(VSOutput input) : SV_Target
{
    uint idx = NonUniformResourceIndex(input.instanceID);
    return sampledTextures[idx].Sample(samplers[0], saturate(input.uv));
}