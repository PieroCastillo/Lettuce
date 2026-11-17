implementing LettuceRendering;

namespace std
{
enum ProceduralShapeGeometryType : uint32_t
{
    Sphere = 0,
    Prism = 1,
    Cylinder = 2
};

struct ProceduralInstanceStorage
{
    float4x4 transform;
    float4x4 invTransform;
    uint32_t geometryType;
    uint32_t geometryIdx;
    uint32_t materialIdx;
    uint32_t flags;
};

struct ProceduralInstanceData
{
    uint32_t count;
    uint32_t sphereCount;
    uint32_t prismCount;
    uint32_t cylinderCount;
    ProceduralInstanceStorage instances[];
};

struct DeferredVSOutput
{
    float4 position : SV_Position;
    float2 uv;
    float3 worldNormal;
    float4 worldTangent; // xyz = tangent, w = handedness (+1/-1)

    nointerpolation uint materialID;
    nointerpolation uint pickID;
};
};