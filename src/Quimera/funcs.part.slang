implementing LettuceQuimera;

__include "types.part.slang";

void UnpackDrawCommandFlags(uint flags, out uint geometryHeap, out uint brushHeap, out uint effectHeap, out bool ignoreTransform, out bool clipped)
{
    geometryHeap = (flags >> 0) & 0xF;
    brushHeap = (flags >> 4) & 0xF;
    effectHeap = (flags >> 8) & 0xF;
    ignoreTransform = ((flags >> 30) & 1) != 0;
    clipped = ((flags >> 31) & 1) != 0;
}

float3x3 InverseTransform2D(float2 position, float2 scale, float2 skew, float2 anchorPoint, float rotation)
{
    float s, c;
    sincos(rotation, s, c);

    float sx = scale.x;
    float sy = scale.y;
    float kx = skew.x;
    float ky = skew.y;

    // A = R * Skew * Scale
    float a = sx * (c - s * ky);
    float b = sy * (c * kx - s);
    float d = sy * (s * kx + c);
    float e = sx * (s + c * ky);

    float invDet = rcp(sx * sy * (1.0 - kx * ky));

    // inv(A)
    float m00 = d * invDet;
    float m01 = -b * invDet;
    float m10 = -e * invDet;
    float m11 = a * invDet;

    // local = inv(A) * screen + anchor - inv(A) * position
    float tx = anchorPoint.x - (m00 * position.x + m01 * position.y);
    float ty = anchorPoint.y - (m10 * position.x + m11 * position.y);

    return float3x3(m00, m01, tx,
                    m10, m11, ty,
                    0.0, 0.0, 1.0);
}

bool IntersectImplicit(uint2 pos, ImplicitGeometryStorage geom, float3x3 invTransform)
{
    // centers using 0.5; next convert to local coords
    float2 screenPos = float2(pos) + 0.5;
    float2 localPos = mul(float3(screenPos, 1.0), transpose(invTransform)).xy;

    float maxRadius = 0.5 * min(geom.w, geom.h);
    // clamps values
    float ctl = clamp(geom.ctl, 0.0, maxRadius);
    float ctr = clamp(geom.ctr, 0.0, maxRadius);
    float cbl = clamp(geom.cbl, 0.0, maxRadius);
    float cbr = clamp(geom.cbr, 0.0, maxRadius);

    float radius = (localPos.x < 0.0) ? ((localPos.y < 0.0) ? cbl : ctl) : ((localPos.y < 0.0) ? cbr : ctr);
    float2 q = abs(localPos) - 0.5 * float2(geom.w, geom.h) + radius;
    // uses SDF
    float sd = length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - radius;

    return sd <= 0.0;
}
