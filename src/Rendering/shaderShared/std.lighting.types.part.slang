implementing LettuceRendering;

__include "std.constants.part";

namespace std.lighting
{
struct SunLightData
{
    float3 direction; // surface to sun direction
    float intensity;
    float3 color;
    float _padding;
};

struct SpotLightStorage
{
    float3 position;
    float range;
    float3 direction; // emission direction
    float intensity;
    float3 color;
    float innerCos;
    float outerCos;

    float3 padding;
};

struct SpotLightData
{
    uint count;
    uint3 padding;
    SpotLightStorage lights[]; // max count: MaxSpotLights
};

struct CascadeShadowMapData
{
    float4 splitDepths; // cascade view-space depth
    float shadowBias;
    float normalBias;
    float texelSize;
    uint cascadeCount;
    uint shadowTextureID;
    float4x4 worldToShadow[MaxCascades]; // max count: MaxCascades
};
};