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

bool IntersectImplicit(float2 localPos, ImplicitGeometryStorage geom)
{
    float maxRadius = 0.5;
    // clamps values
    float ctl = clamp(geom.ctl, 0.0, maxRadius);
    float ctr = clamp(geom.ctr, 0.0, maxRadius);
    float cbl = clamp(geom.cbl, 0.0, maxRadius);
    float cbr = clamp(geom.cbr, 0.0, maxRadius);

    float radius = (localPos.x < 0.0) ? ((localPos.y < 0.0) ? cbl : ctl) : ((localPos.y < 0.0) ? cbr : ctr);
    float2 q = abs(localPos) - 0.5 + radius;
    // uses SDF
    float sd = length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - radius;

    return sd <= 0.0;
}