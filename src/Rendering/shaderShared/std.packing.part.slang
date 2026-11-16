implementing LettuceRendering;

namespace std
{
uint packUV(float2 uv)
{
    uint2 u = uint2(round(saturate(uv) * 65535.0));
    return u.x | (u.y << 16);
}

uint packDerivatives(float dx, float dy)
{
    uint x = f32tof16(dx) & 0xFFFF;
    uint y = f32tof16(dy) & 0xFFFF;

    return x | (y << 16);
}

uint packRotationMaterial(float angleX, float angleY, uint materialID)
{
    var normalizedAngles = float2(angleX, angleY) * (1.0 / (2.0 * Pi)) + 0.5;
    var angles = uint2(round(saturate(normalizedAngles) * 255.0));

    return (angles.x) | (angles.y << 8) | ((materialID & 0xFFFF) << 16);
}

uint packTangent(float4 q)
{
    uint4 encoded = uint4(round(saturate(q * 0.5 + 0.5) * 255.0));
    return encoded.x | (encoded.y << 8) | (encoded.z << 16) | (encoded.w << 24);
}

float2 unpackUnorm2x16(uint packed)
{
    uint x = packed & 0xFFFF;
    uint y = (packed >> 16) & 0xFFFF;
    return float2(x, y) / 65535.0;
}

float4 unpackUnorm4x8(uint packed)
{
    uint x = packed & 0xFF;
    uint y = (packed >> 8) & 0xFF;
    uint z = (packed >> 16) & 0xFF;
    uint w = (packed >> 24) & 0xFF;
    return float4(x, y, z, w) / 255.0;
}
};