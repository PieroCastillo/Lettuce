implementing LettuceRendering;

__include "std.procedural.part.slang";
__include "std.constants.part.slang";

namespace std
{
struct DeferredPSOutput
{
    uint4 gbufferOutput : SV_Target0;
    uint pickOutput : SV_Target1;
};

DeferredPSOutput deferredFragBase(DeferredVSOutput input)
{
    var uvDdx = ddx(input.uv);
    var uvDdy = ddy(input.uv);

    var lenDdx = length(uvDdx);
    var lenDdy = length(uvDdy);

    // avoid atan2 UB for degenerate UVs.
    var angleX = atan2(uvDdx.y, uvDdx.x);
    var angleY = atan2(uvDdy.y, uvDdy.x);

    // tangent rebuild
    var N = normalize(input.worldNormal);
    var T = input.worldTangent.xyz;

    T = normalize(T - N * dot(N, T)); // gram schmidt orthogonalization

    var handedness = input.worldTangent.w < 0.0 ? -1.0 : 1.0;
    var B = normalize(cross(N, T)) * handedness;
    var tangentQuaternion = tbnToQuaternion(T, B, N);

    // gbuffer compression
    var packedUV = packUV(input.uv);
    var packedDerivatives = packDerivatives(min(lenDdx, 65504.0), min(lenDdy, 65504.0));
    var packedRotationMaterial = packRotationMaterial(angleX, angleY, input.materialID);
    var packedTangent = packTangent(tangentQuaternion);

    // material render target output
    DeferredPSOutput output;
    output.gbufferOutput = uint4(packedUV, packedDerivatives, packedRotationMaterial, packedTangent);
    output.pickOutput = input.pickID;

    return output;
}
};