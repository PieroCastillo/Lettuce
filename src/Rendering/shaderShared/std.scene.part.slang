implementing LettuceRendering;

namespace std
{
struct SceneViewData
{
    float4x4 viewProj;
    float4x4 invViewProj;
    float3 cameraPosition;
    uint32_t instanceCount;
    float3 cameraForward;
    float _padding;
};

struct MaterialShadingData
{
    uint32_t depthTextureIdx;
    uint32_t gbufferTextureIdx;
};
};