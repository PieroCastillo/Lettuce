implementing LettuceRendering;

const static float Pi = radians(180);
const static float InvPi = 1 / radians(180);
const static float Epsilon = 1e-7;

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - saturate(cosTheta), 5.0);
}

float distributionGGX(float NdotH, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;

    float d = NdotH * NdotH * (alpha2 - 1.0) + 1.0;

    return alpha2 * rcp(max(Pi * d * d, Epsilon));
}

float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) * 0.125;

    return NdotV * rcp(max(NdotV * (1.0 - k) + k, Epsilon));
}

float geometrySmith(float NdotV, float NdotL, float roughness)
{
    return geometrySchlickGGX(NdotV, roughness) * geometrySchlickGGX(NdotL, roughness);
}

float3 evaluateBRDF(float3 N, float3 V, float3 L, float3 albedo, float metallic, float roughness)
{
    var H = normalize(V + L);

    var NdotV = max(dot(N, V), Epsilon * 100);
    var NdotL = max(dot(N, L), 0.0);
    var NdotH = max(dot(N, H), 0.0);
    var HdotV = max(dot(H, V), 0.0);

    if (NdotL <= 0.0)
        return 0.0;

    roughness = clamp(roughness, 0.045, 1.0);
    metallic = saturate(metallic);

    var F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);
    var F = fresnelSchlick(HdotV, F0);
    var D = distributionGGX(NdotH, roughness);
    var G = geometrySmith(NdotV, NdotL, roughness);

    var specular = (D * G * F) * rcp(max(4.0 * NdotV * NdotL, Epsilon * 10));
    var kD = (1.0 - F) * (1.0 - metallic);
    var diffuse = kD * albedo * InvPi;

    return diffuse + specular;
}