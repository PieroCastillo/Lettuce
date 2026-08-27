module LettuceQuimera;

__include "funcs.part.slang";
__include "types.part.slang";

Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D<float4> storageTextures[];

const static uint computeNumThreadsX = 32;
const static uint tileSize = 16;
const static uint tileMaxCommands = 32;
const static uint samplerDefaultIndex = 0;
const static uint samplerSdfIndex = 1;

[[vk::push_constant]]
PushConstants pc;

struct VSOutput
{
    float4 clipPos : SV_Position;
    float2 localPos;
    float2 uv;
    int drawCommandIdx;
};

struct PSOutput
{
    float4 color : SV_Target0;
    uint32_t pick : SV_Target1;
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

    // var invTransform = InverseTransform2D(layout.position, layout.scale, layout.skew, layout.anchorPoint, layout.rotation);

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
    var glyphs = reinterpret<Ptr<GlyphGeometryStorage>>(pc.glyphGeometriesAddr);

    var drawCmd = drawCmds[instanceID];
    var transform = transforms[drawCmd.layoutIdx];
    var layout = layouts[drawCmd.layoutIdx];

    uint brushHeap, geometryHeap, effectHeap;
    bool ignoreTransform, clipped;

    UnpackDrawCommandFlags(drawCmd.flags, geometryHeap, brushHeap, effectHeap, ignoreTransform, clipped);

    float2 corners[6] = {
        float2(-0.5, 0.5),
        float2(0.5, 0.5),
        float2(0.5, -0.5),

        float2(-0.5, 0.5),
        float2(0.5, -0.5),
        float2(-0.5, -0.5),
    };

    float2 uvs[6] = {
        float2(0.0, 1.0), // Bottom-Left  (-0.5,  0.5)
        float2(1.0, 1.0), // Bottom-Right ( 0.5,  0.5)
        float2(1.0, 0.0), // Top-Right    ( 0.5, -0.5)

        float2(0.0, 1.0), // Bottom-Left  (-0.5,  0.5)
        float2(1.0, 0.0), // Top-Right    ( 0.5, -0.5)
        float2(0.0, 0.0), // Top-Left     (-0.5, -0.5)
    };
    var localPos = corners[vertexID % 6];
    var uv = uvs[vertexID % 6];

    float s, c;
    sincos(layout.rotation, s, c);

    var rot = float2x2(c, -s, s, c);
    var skew = float2x2(1, layout.skew.y, layout.skew.x, 1);

    // var local = localPos - layout.anchorPoint;
    // local = mul(skew, local);
    // local = mul(rot, local);
    // local *= layout.scale;

    // // normalized local coords to surface coords
    // var surfPos = local + (0.5 * layout.scale) + layout.position;
    float2 surfPos;

    if (geometryHeap == 1)
    {
        var glyph = glyphs[drawCmd.geometryIdx];
        var offsetX = asfloat(drawCmd.reserved1);
        var offsetY = asfloat(drawCmd.reserved2);

        var glyphSize = float2(glyph.bitmapWidth, glyph.bitmapHeight);
        var glyphOffset = float2(offsetX + glyph.bitmapLeft, offsetY - glyph.bitmapTop);

        var localTextPos = (uv * glyphSize) + glyphOffset;

        var local = localTextPos - layout.anchorPoint;
        local = mul(skew, local);
        local = mul(rot, local);

        surfPos = (local * layout.scale) + layout.position;
    }
    else
    {
        var local = localPos - layout.anchorPoint;
        local = mul(skew, local);
        local = mul(rot, local);
        local *= layout.scale;

        // normalized local coords to surface coords
        surfPos = local + (0.5 * layout.scale) + layout.position;
    }

    // surface coords to vulkan NDC
    var clip = mul(float2x2(2 * rcp(surfaceData.width), 0, 0, 2 * rcp(surfaceData.height)), surfPos) - float2(1);

    VSOutput output;
    output.localPos = localPos;
    output.uv = uvs[vertexID % 6];
    output.clipPos = float4(clip, drawCmd.zOrder * rcp(99999), 1);
    output.drawCommandIdx = instanceID;

    return output;
}

[require(fragment, spirv_1_6)]
[shader("fragment")]
PSOutput fragMain(VSOutput input)
{
    var drawCmds = reinterpret<Ptr<DrawCommand>>(pc.drawCommandsAddr);
    var implicitGeometries = reinterpret<Ptr<ImplicitGeometryStorage>>(pc.implicitGeometriesAddr);
    var solidColorBrushes = reinterpret<Ptr<SolidColorBrushStorage>>(pc.solidColorBrushesAddr);
    var glyphs = reinterpret<Ptr<GlyphGeometryStorage>>(pc.glyphGeometriesAddr);

    var drawCmd = drawCmds[input.drawCommandIdx];

    uint brushHeap, geometryHeap, effectHeap;
    bool ignoreTransform, clipped;

    UnpackDrawCommandFlags(drawCmd.flags, geometryHeap, brushHeap, effectHeap, ignoreTransform, clipped);

    if (brushHeap != 0)
        discard;

    var brush = solidColorBrushes[drawCmd.brushIdx];
    var geometry = implicitGeometries[drawCmd.geometryIdx];

    PSOutput output;
    output.color = brush.color;
    output.pick = (input.drawCommandIdx + 1) | (1 << 31); // first zero

    switch (geometryHeap)
    {
    case 0:
        if (!IntersectImplicit(input.localPos, implicitGeometries[drawCmd.geometryIdx]))
            discard;
        break;
    case 1:
    {
        var glyph = glyphs[drawCmd.geometryIdx];
        var sd = sampledTextures[glyph.descriptorIdx].Sample(samplers[samplerSdfIndex], input.uv).x;

        var smoothing = fwidth(sd) * 0.5;
        var alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, sd);

        // needs hardware color blending enable
        output.color = float4(brush.color.xyz, alpha);
        // output.color = float4(alpha, alpha, alpha, alpha);
    }
    break;
    default:
        discard;
    }

    return output;
}
