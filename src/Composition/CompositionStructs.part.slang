implementing LettuceComposition;

const static uint maxElementsPerTile = 32;

enum class BrushType : uint8_t
{
    SolidColor,     // Flat color surface
    LinearGradient, // Linear color interpolation
    RadialGradient, // Radial color interpolation
    Texture,        // Textured surface
    NormalMap,      // Surface with normal mapping for lighting
    Metallic,       // Metallic surface
    Roughness,      // Roughness variation
    Noise,          // Procedural noise texture
    Distortion,     // UV distortion
};

enum AnimatableProperties : uint8_t
{
    // Visual Props
    Position,
    Size,
    Scale,
    Rotation,
    AnchorPoint,
    Opacity,
    CornerRadius,
    BorderWidth,
    // Light Props
    Lightfloat4,
    LightIntensity,
    LightPosition,
    LightDirection,
    // Material Props
    Brushfloat4,
    BrushSecondaryfloat4,
    BrushRoughness,
    BrushDistortionStrength,
    BrushNoiseScale,
    BrushNoiseIntensity,
};

enum EasingMode : uint8_t
{
    Linear,
    EaseIn,
    EaseOut,
    EaseInOut,
    EaseInBack,
    EaseOutBack,
    EaseInOutBack,
    EaseOutBounce,
    EaseOutElastic,
    CubicBezier,
};

enum LightType : uint8_t
{
    Ambient,
    Point,
    Spot,
};

struct AnimationInstanceGPUData
{
    AnimatableProperties objectProperty;
    uint32_t objectIdx;
    uint32_t animationTokenIdx;
    float startValue[4];
    float targetValue[4];
    bool naturalMotion;
    double startTime;
    double duration;
};

struct AnimationTokenGPUData
{
    EasingMode easing;
    float bezierX1;
    float bezierY1;
    float bezierX2;
    float bezierY2;

    float stiffness;
    float damping;
    float mass;
};

struct VisualGPUData
{
    float2 Position;
    float2 Size;
    float2 Scale;
    float Rotation;
    float2 AnchorPoint;
    float Opacity;
    float CornerRadius;
    float BorderWidth;
};

struct LightGPUData
{
    LightType type;
    float4 color;
    float intensity;
    float3 position;
    float3 direction;
};

struct FrameGPUData
{
    float4x4 viewProj;
    float2 viewportSize;
    float2 padding;
};

struct TileInstance
{
    uint32_t count;
    uint32_t visualIDs[maxElementsPerTile];
};