implementing LettuceRendering;

Ptr<T> loadAddr<T>(uint64_t addr, uint32_t offset = 0)
{
    return reinterpret<Ptr<T>>(addr + (offset * sizeof(T)));
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

float3x3 quaternionToTBN(float4 q)
{
    float x = q.x, y = q.y, z = q.z, w = q.w;
    float x2 = x + x, y2 = y + y, z2 = z + z;
    float xx = x * x2, xy = x * y2, xz = x * z2;
    float yy = y * y2, yz = y * z2, zz = z * z2;
    float wx = w * x2, wy = w * y2, wz = w * z2;

    float3 T = float3(1.0 - (yy + zz), xy + wz, xz - wy);
    float3 B = float3(xy - wz, 1.0 - (xx + zz), yz + wx);
    float3 N = float3(xz + wy, yz - wx, 1.0 - (xx + yy));

    return float3x3(T, B, N);
}