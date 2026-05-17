implementing LettuceComposition;

const static uint maxGradientStopsCount = 8;

struct SolidColorMaterialGPU
{
    float4 color;
};

struct GradientStopGPU
{
    float4 color;
    float position;
    float3 padding;
};

struct GradientPointGPU
{
    float4 color;
    float2 position;
    float2 padding0;
};

struct LinearGradientMaterialGPU
{
    float2 startPoint;
    float2 endPoint;

    uint stopCount;
    float3 padding;

    GradientStopGPU stops[maxGradientStopsCount];
};

struct RadialGradientMaterialGPU
{
    float2 center;
    float2 radius;

    float2 focalPoint;
    float2 padding0;

    uint stopCount;
    float3 padding1;

    GradientStopGPU stops[maxGradientStopsCount];
};

struct MeshGradientBrushGPU
{
    uint rows;
    uint cols;
    uint pointCount;
    float padding;

    GradientPointGPU points[maxGradientStopsCount * maxGradientStopsCount];
};

struct TextureMaterialGPU
{
    float4 tintColor;

    float2 uvOffset;
    float2 uvScale;

    uint textureIndex;
    uint samplerIndex;

    uint tilingMode;
    uint padding;
};

struct GlassMaterialGPU
{
    float4 tintColor;

    float opacity;
    float blurStrength;
    float highlightIntensity;
    float refractionStrength;

    uint backgroundTextureIndex;
    uint samplerIndex;

    float2 padding;
};

struct AcrylicMaterialGPU
{
    float4 tintColor;

    float tintOpacity;
    float blurStrength;
    float noiseOpacity;
    float padding;

    uint backgroundTextureIndex;
    uint noiseTextureIndex;

    uint samplerIndex;
    uint padding2;
};

struct NoiseMaterialGPU
{
    float4 noiseColor;

    float scale;
    float intensity;
    float animationTime;
    float padding;

    uint noiseTextureIndex;
    uint samplerIndex;

    float2 padding2;
};


struct DistortionMaterialGPU
{
    float2 padding;
};