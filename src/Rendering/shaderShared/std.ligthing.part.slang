implementing LettuceRendering;

__include "std.lighting.types.part.slang";
__include "std.lighting.aux.part.slang";

namespace std.lighting
{
float3 evaluateSun(SunLightData sun, float3 N, float3 V, float3 albedo, float metallic, float roughness, float shadow)
{
    var L = normalize(sun.direction);
    var NdotL = saturate(dot(N, L));
    var brdf = evaluateBRDF(N, V, L, albedo, metallic, roughness);
    return brdf * sun.color * sun.intensity * NdotL * shadow;
}

float3 evaluateSpot(Ptr<SpotLightData> spots, uint index, float3 worldPos, float3 N, float3 V, float3 albedo, float metallic, float roughness)
{
    SpotLightStorage light = spots[0].lights[index]; // temp fix, i'm fighting with the compiler due to error[E99997]
    var toLight = light.position - worldPos;

    var distanceSq = dot(toLight, toLight);
    var distance = sqrt(distanceSq);

    var L = toLight * rcp(max(distance, 1e-5));

    var NdotL = saturate(dot(N, L));

    if (NdotL <= 0.0 || distance >= light.range)
        return 0.0;

    var x = distance * rcp(light.range); // smooth range attenuation
    var rangeFade = saturate(1.0 - x * x * x * x);

    rangeFade *= rangeFade;

    var coneCos = dot(-L, normalize(light.direction)); // spotlight cone
    var cone = smoothstep(light.outerCos, light.innerCos, coneCos);
    var attenuation = rangeFade * rcp(max(distanceSq, 1e-4));
    var brdf = evaluateBRDF(N, V, L, albedo, metallic, roughness);

    return brdf * light.color * light.intensity * attenuation * cone * NdotL;
}
};