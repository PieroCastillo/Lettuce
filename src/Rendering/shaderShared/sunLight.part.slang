implementing LettuceRendering;

__include "typesLighting.part.slang";
__include "directIllumBrdfGGX.part.slang";

float3 evaluateSun(SunLightData sun, float3 N, float3 V, float3 albedo, float metallic, float roughness, float shadow)
{
    var L = normalize(sun.direction);
    var NdotL = saturate(dot(N, L));
    var brdf = evaluateBRDF(N, V, L, albedo, metallic, roughness);
    return brdf * sun.color * sun.intensity * NdotL * shadow;
}