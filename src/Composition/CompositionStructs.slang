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
    // Brush Props
    Brushfloat4,
    BrushSecondaryfloat4,
    BrushRoughness,
    BrushDistortionStrength,
    BrushNoiseScale,
    BrushNoiseIntensity,
    // Effects Props
    EffectIntensity,
    Effectfloat4,
    EffectOffset,
    EffectRadius,
    EffectBlurAmount,
    EffectTintOpacity,
    EffectTintfloat4,
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

enum EffectType : uint8_t
{
    Blur,        // Gaussian blur
    DropShadow,  // Shadow overlay
    InnerShadow, // Inset shadow
    Glow,        // Outer glow
    Saturation,  // Color saturation adjustment
    Brightness,  // Brightness adjustment
    Contrast,    // Contrast adjustment
    HueRotate,   // Hue rotation
    Acrylic,     // Frosted glass effect (blur + tint + noise)
    Glass,       // Glass/transparency effect
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

struct BrushGPUData
{
    BrushType type;
    float4 color;
    float4 secondaryfloat4;
    uint32_t textureIdx;
    float distortionStrength;
    float noiseScale;
    float noiseIntensity;
};

struct LightGPUData
{
    LightType type;
    float4 color;
    float intensity;
    float3 position;
    float3 direction;
};

struct EffectGPUData
{
    EffectType type;
    float intensity;
    float4 color;
    float2 offset;
    float radius;
    float blurAmount;
    float tintOpacity;
    float4 tintfloat4;
};

struct FrameGPUData
{
    float4x4 viewProj;
    float2 viewportSize;
    float2 padding;
};
