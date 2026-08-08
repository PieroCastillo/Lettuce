module LettuceQuimera;

__include "funcs.part.slang";
__include "types.part.slang";

Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D<float4> storageTextures[];

const static uint computeNumThreadsX = 32;
const static uint tileSize = 16;
const static uint tileMaxCommands = 32;

[[vk::push_constant]]
PushConstants pc;

struct VSOutput
{
    float4 clipPos : SV_Position;
    float2 localPos;
    int drawCommandIdx;
};

[require(compute, spirv_1_6)]
[numthreads(computeNumThreadsX, 1, 1)]
void preprocessMain(uint3 gid: SV_DispatchThreadID)
{
    var surfaceData = reinterpret<Ptr<SurfaceData>>(pc.surfaceDataAddr)[0];

    var drawCmds = reinterpret<Ptr<DrawCommand>>(pc.drawCommandsAddr);
    var layouts = reinterpret<Ptr<LayoutStorage>>(pc.layoutsAddr);
    var transforms = reinterpret<Ptr<float3x3>>(pc.scratchTransformsAddr);
    var invTransforms = reinterpret<Ptr<float3x3>>(pc.scratchInvTransformsAddr);

    uint i = gid.x;

    if (i >= surfaceData.drawCommandCount)
        return;

    var cmd = drawCmds[i];
    var layout = layouts[cmd.layoutIdx];

    //var invTransform = InverseTransform2D(layout.position, layout.scale, layout.skew, layout.anchorPoint, layout.rotation);

    // transforms[cmd.layoutIdx] = invTransform;
    // invTransforms[cmd.layoutIdx] = InvertMatrix(invTransform);
}

[require(vertex, spirv_1_6)]
[shader("vertex")]
VSOutput vertMain(uint vertexID: SV_VertexID, uint instanceID: SV_InstanceID)
{
    var surfaceData = reinterpret<Ptr<SurfaceData>>(pc.surfaceDataAddr)[0];
    var drawCmds = reinterpret<Ptr<DrawCommand>>(pc.drawCommandsAddr);
    var layouts = reinterpret<Ptr<LayoutStorage>>(pc.layoutsAddr);
    var transforms = reinterpret<Ptr<float3x3>>(pc.scratchTransformsAddr);

    var drawCmd = drawCmds[instanceID];
    var transform = transforms[drawCmd.layoutIdx];
    var layout = layouts[drawCmd.layoutIdx];

    float2 corners[6] = {
        float2(-0.5, 0.5),
        float2(0.5, 0.5),
        float2(0.5, -0.5),
        
        float2(-0.5, 0.5),
        float2(0.5, -0.5),
        float2(-0.5, -0.5),
    };

    var localPos = corners[vertexID % 6];

    float s, c;
    sincos(layout.rotation, s, c);

    var rot = float2x2(  c, -s,       s, c    );
    var skew = float2x2(1, layout.skew.y,  layout.skew.x, 1);

    var local = localPos - layout.anchorPoint;
    local = mul(skew, local);
    local = mul(rot, local);
    local *= layout.scale;

    // normalized local coords to surface coords
    var surfPos = local + (0.5 * layout.scale) + layout.position;

    // surface coords to vulkan NDC
    var clip = mul(float2x2(2 * rcp(surfaceData.width), 0, 0, 2 * rcp(surfaceData.height)), surfPos) - float2(1);

    VSOutput output;
    output.localPos = localPos;
    output.clipPos = float4(clip, drawCmd.zOrder * rcp(99999), 1);
    output.drawCommandIdx = instanceID;

    return output;
}

[require(fragment, spirv_1_6)]
[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target
{
    var drawCmds = reinterpret<Ptr<DrawCommand>>(pc.drawCommandsAddr);
    var implicitGeometries = reinterpret<Ptr<ImplicitGeometryStorage>>(pc.implicitGeometriesAddr);
    var solidColorBrushes = reinterpret<Ptr<SolidColorBrushStorage>>(pc.solidColorBrushesAddr);

    var drawCmd = drawCmds[input.drawCommandIdx];

    uint brushHeap, geometryHeap, effectHeap;
    bool ignoreTransform, clipped;

    UnpackDrawCommandFlags(drawCmd.flags, geometryHeap, brushHeap, effectHeap, ignoreTransform, clipped);

    if (brushHeap != 0 || geometryHeap != 0)
        discard;

    var brush = solidColorBrushes[drawCmd.brushIdx];
    var geometry = implicitGeometries[drawCmd.geometryIdx];

    if (!IntersectImplicit(input.localPos, geometry))
        discard;

    return brush.color;
}