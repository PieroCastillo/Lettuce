implementing LettuceRendering;

__include "funcs.part.slang";
__include "typesLighting.part.slang";
__include "directIllumBrdfGGX.part.slang";

float sampleCSM(Texture2D<float4> textures[], CascadeShadowMapData csm, float3 worldPos, float3 N, float3 L, float viewDepth)
{
    var cascade = csm.cascadeCount;

    [unroll]
    for (uint i = 0; i < MaxCascades; ++i)
    {
        if (i < csm.cascadeCount && viewDepth <= csm.splitDepths[i])
        {
            cascade = i;
            break;
        }
    }

    if (cascade >= csm.cascadeCount)
        return 1.0;

    var biasedPosition = worldPos + N * csm.normalBias;
    var clip = mul(csm.worldToShadow[cascade], float4(biasedPosition, 1.0));
    var projected = clip.xyz / clip.w;
    var uv = projected.xy * 0.5 + 0.5;
    uv.y = 1.0 - uv.y;

    if (any(uv < 0.0) || any(uv > 1.0) || projected.z < 0.0 || projected.z > 1.0)
        return 1.0;

    var NdotL = saturate(dot(N, L));
    var bias = max(csm.shadowBias * (1.0 - NdotL), csm.shadowBias * 0.25);
    var referenceDepth = projected.z - bias;

    var center = int2(uv / csm.texelSize);
    float visibility = 0.0;

    [unroll]
    for (int y = -1; y <= 1; ++y)
    {
        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            int2 pixel = center + int2(x, y);
            var storedDepth = textures[csm.shadowTextureID].Load(int3(pixel, 0)).r;

            visibility += referenceDepth <= storedDepth ? 1.0 : 0.0;
        }
    }

    return visibility / 9.0;
}
