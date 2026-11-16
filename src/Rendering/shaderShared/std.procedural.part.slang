implementing LettuceRendering;

namespace std
{
struct DeferredVSOutput
{
    float4 position : SV_Position;
    float2 uv;
    float3 worldNormal;
    float4 worldTangent; // xyz = tangent, w = handedness (+1/-1)

    nointerpolation uint materialID;
    nointerpolation uint pickID;
};
};