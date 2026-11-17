implementing LettuceRendering;

namespace std
{
struct SphereStorage
{
    float radius;
    uint32_t longitudeSegments;
    uint32_t latitudeSegments;
    uint32_t _padding;
};

struct PrismStorage
{
    float radius;
    float height;
    uint32_t sides;
    uint32_t _padding;
};

struct CylinderStorage
{
    float radiusTop;
    float radiusBottom;
    float height;
    uint32_t radialSegments;
};

uint32_t getSphereVertexCount(SphereStorage sphere)
{
    return 6 * sphere.longitudeSegments * sphere.latitudeSegments;
}

uint32_t getPrismVertexCount(PrismStorage prism)
{
    return 12 * prism.sides;
}

uint32_t getCylinderVertexCount(CylinderStorage cylinder)
{
    return 12 * cylinder.radialSegments;
}
};