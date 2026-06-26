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

bool IntersectImplicit(uint2 pos, ImplicitGeometryStorage geom, float3x3 invTransform)
{
    // centers using 0.5; next convert to local coords
    var localPos = mul(invTransform, float3(pos + float2(0.5), 1)).xy;
    // uses SDF
    var radious = (localPos.x < 0) ? ((localPos.y < 0) ? geom.cbl : geom.ctl) : ((localPos.y < 0) ? geom.cbr : geom.ctr);

    var q = float2(abs(localPos.x) - (0.5 * geom.w) + radious, abs(localPos.y) - (0.5 * geom.h) + radious);
    var sd = length(max(q, float2(0, 0))) + min(max(q.x, q.y), 0) - radious;

    return sd <= 0;
}