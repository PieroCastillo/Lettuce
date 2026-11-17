implementing LettuceRendering;

__include "std.constants.part.slang";
__include "std.shapes.part.slang";

namespace std
{
float2 getQuadCorner(uint vertex)
{
    switch (vertex)
    {
    case 0:
        return float2(0, 0);
    case 1:
        return float2(1, 0);
    case 2:
        return float2(1, 1);
    case 3:
        return float2(1, 1);
    case 4:
        return float2(0, 1);
    default:
        return float2(0, 0);
    }
}

float3 getPolygonPoint(float radius, float angle, float y)
{
    return float3(radius * cos(angle), y, radius * sin(angle));
}

void generateSphereVertex(SphereStorage sphere, uint vertexID, out float3 position, out float3 normal, out float3 tangent, out float2 uv)
{
    uint longitude = sphere.longitudeSegments;
    uint latitude = sphere.latitudeSegments;

    uint cell = vertexID / 6;
    uint triangleVertex = vertexID % 6;

    uint x = cell % longitude;
    uint y = cell / longitude;

    float2 corner = getQuadCorner(triangleVertex);

    float u = (float(x) + corner.x) / float(longitude);
    float v = (float(y) + corner.y) / float(latitude);

    float theta = u * 2.0 * std.Pi;
    float phi = v * std.Pi;

    float sinPhi = sin(phi);
    float cosPhi = cos(phi);

    float sinTheta = sin(theta);
    float cosTheta = cos(theta);

    normal = float3(sinPhi * cosTheta, cosPhi, sinPhi * sinTheta);
    position = normal * sphere.radius;
    tangent = float3(-sinTheta, 0, cosTheta);

    uv = float2(u, 1.0 - v);
}

void generatePrismVertex(PrismStorage prism, uint vertexID, out float3 position, out float3 normal, out float3 tangent, out float2 uv)
{
    uint sides = prism.sides;

    float halfHeight = prism.height * 0.5;
    uint sideVertexCount = sides * 6;

    // side faces
    if (vertexID < sideVertexCount)
    {
        uint side = vertexID / 6;
        uint vertex = vertexID % 6;

        float2 corner = getQuadCorner(vertex);

        float angle0 = float(side) * (2.0 * std.Pi / float(sides));
        float angle1 = float(side + 1) * (2.0 * std.Pi / float(sides));

        float3 bottomA = getPolygonPoint(prism.radius, angle0, -halfHeight);
        float3 bottomB = getPolygonPoint(prism.radius, angle1, -halfHeight);
        float3 topA = getPolygonPoint(prism.radius, angle0, halfHeight);
        float3 topB = getPolygonPoint(prism.radius, angle1, halfHeight);

        switch (vertex)
        {
        case 0:
            position = bottomA;
            break;
        case 1:
            position = bottomB;
            break;
        case 2:
            position = topB;
            break;
        case 3:
            position = topB;
            break;
        case 4:
            position = topA;
            break;
        default:
            position = bottomA;
            break;
        }

        float middleAngle = (angle0 + angle1) * 0.5;

        normal = float3(cos(middleAngle), 0, sin(middleAngle));
        tangent = normalize(bottomB - bottomA);
        uv = float2((float(side) + corner.x) / float(sides), corner.y);

        return;
    }

    // caps
    uint capVertex = vertexID - sideVertexCount;
    uint side = capVertex / 6;
    uint vertex = capVertex % 6;

    float angle0 = float(side) * (2.0 * std.Pi / float(sides));
    float angle1 = float(side + 1) * (2.0 * std.Pi / float(sides));

    float3 pointA = getPolygonPoint(prism.radius, angle0, halfHeight);
    float3 pointB = getPolygonPoint(prism.radius, angle1, halfHeight);

    if (capVertex < sides * 3)
    {
        // top caps
        switch (vertex)
        {
        case 0:
            position = float3(0, halfHeight, 0);
            break;
        case 1:
            position = pointB;
            break;
        default:
            position = pointA;
            break;
        }

        normal = float3(0, 1, 0);
    }
    else
    {
        // bottom cap
        pointA.y = -halfHeight;
        pointB.y = -halfHeight;

        switch (vertex)
        {
        case 0:
            position = float3(0, -halfHeight, 0);
            break;
        case 1:
            position = pointA;
            break;
        default:
            position = pointB;
            break;
        }
        normal = float3(0, -1, 0);
    }

    tangent = float3(1, 0, 0);
    uv = position.xz / (2.0 * max(prism.radius, 0.0001)) + 0.5;
}

void generateCylinderVertex(CylinderStorage cylinder, uint vertexID, out float3 position, out float3 normal, out float3 tangent, out float2 uv)
{
    var segments = cylinder.radialSegments;
    var halfHeight = cylinder.height * 0.5;
    var sideVertexCount = segments * 6;

    // side faces
    if (vertexID < sideVertexCount)
    {
        uint segment = vertexID / 6;
        uint vertex = vertexID % 6;

        var corner = getQuadCorner(vertex);

        var angle0 = float(segment) * (2.0 * std.Pi / float(segments));
        var angle1 = float(segment + 1) * (2.0 * std.Pi / float(segments));

        var bottomA = getPolygonPoint(cylinder.radiusBottom, angle0, -halfHeight);
        var bottomB = getPolygonPoint(cylinder.radiusBottom, angle1, -halfHeight);
        var topA = getPolygonPoint(cylinder.radiusTop, angle0, halfHeight);
        var topB = getPolygonPoint(cylinder.radiusTop, angle1, halfHeight);

        switch (vertex)
        {
        case 0:
            position = bottomA;
            break;
        case 1:
            position = bottomB;
            break;
        case 2:
            position = topB;
            break;
        case 3:
            position = topB;
            break;
        case 4:
            position = topA;
            break;
        default:
            position = bottomA;
            break;
        }

        float angle = lerp(angle0, angle1, 0.5);
        float slope = (cylinder.radiusBottom - cylinder.radiusTop) / max(cylinder.height, 0.0001);

        normal = normalize(float3(cos(angle), slope, sin(angle)));
        tangent = float3(-sin(angle),0, cos(angle));

        uv = float2((float(segment) + corner.x) / float(segments),corner.y);

        return;
    }

    // Ccps
    uint capVertex = vertexID - sideVertexCount;
    uint segment = capVertex / 6;
    uint vertex = capVertex % 6;

    var angle0 = float(segment) * (2.0 * std.Pi / float(segments));
    var angle1 = float(segment + 1) * (2.0 * std.Pi / float(segments));

    var top = capVertex < segments * 3;
    var radius = top? cylinder.radiusTop: cylinder.radiusBottom;

    var y = top ? halfHeight : -halfHeight;

    var pointA = getPolygonPoint(radius, angle0, y);
    var pointB = getPolygonPoint(radius, angle1, y);

    if (top)
    {
        switch (vertex)
        {
        case 0:
            position = float3(0, y, 0);
            break;
        case 1:
            position = pointB;
            break;
        default:
            position = pointA;
            break;
        }

        normal = float3(0, 1, 0);
    }
    else
    {
        switch (vertex)
        {
        case 0:
            position = float3(0, y, 0);
            break;
        case 1:
            position = pointA;
            break;
        default:
            position = pointB;
            break;
        }

        normal = float3(0, -1, 0);
    }

    tangent = float3(1, 0, 0);
    uv = position.xz / (2.0 * max(radius, 0.0001)) + 0.5;
}
};