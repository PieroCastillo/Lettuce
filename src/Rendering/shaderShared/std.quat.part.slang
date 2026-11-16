implementing LettuceRendering;

namespace std
{
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

float4 tbnToQuaternion(float3 T, float3 B, float3 N)
{
    // TBN matrix, columns = T, B, N
    float m00 = T.x;
    float m01 = B.x;
    float m02 = N.x;

    float m10 = T.y;
    float m11 = B.y;
    float m12 = N.y;

    float m20 = T.z;
    float m21 = B.z;
    float m22 = N.z;

    float4 q;
    float trace = m00 + m11 + m22;

    if (trace > 0.0)
    {
        float s = sqrt(trace + 1.0) * 2.0;

        q.w = 0.25 * s;
        q.x = (m21 - m12) / s;
        q.y = (m02 - m20) / s;
        q.z = (m10 - m01) / s;
    }
    else if (m00 > m11 && m00 > m22)
    {
        float s = sqrt(1.0 + m00 - m11 - m22) * 2.0;

        q.w = (m21 - m12) / s;
        q.x = 0.25 * s;
        q.y = (m01 + m10) / s;
        q.z = (m02 + m20) / s;
    }
    else if (m11 > m22)
    {
        float s = sqrt(1.0 + m11 - m00 - m22) * 2.0;

        q.w = (m02 - m20) / s;
        q.x = (m01 + m10) / s;
        q.y = 0.25 * s;
        q.z = (m12 + m21) / s;
    }
    else
    {
        float s = sqrt(1.0 + m22 - m00 - m11) * 2.0;

        q.w = (m10 - m01) / s;
        q.x = (m02 + m20) / s;
        q.y = (m12 + m21) / s;
        q.z = 0.25 * s;
    }

    q = normalize(q);

    // q and -q represent the same rotation.
    // Use a consistent hemisphere for quantization.
    if (q.w < 0.0)
        q = -q;

    return q;
}
};