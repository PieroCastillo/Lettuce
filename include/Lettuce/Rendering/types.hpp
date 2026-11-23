/*
Created by @PieroCastillo on 2026-05-17
*/
#ifndef LETTUCE_RENDERING_TYPES_HPP
#define LETTUCE_RENDERING_TYPES_HPP

// standard headers
#include <cstdint>

// project headers
#include "../Core/basicTypes.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Rendering
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
        uint32_t depthTextureIdx; // must be on sampled texture heap
        uint32_t gbufferTextureIdx; // must be on sampled texture heap
    };

    struct ClusterStorage
    {
        float4 cone;
        float4 aabbMin;
        float4 aabbMax;
        float4 centroid;

        uint32_t materialHeap;
        uint32_t materialID;

        uint32_t positionsBaseOffset;
        uint32_t positionsCount;
        uint32_t normalsBaseOffset;
        uint32_t normalsCount;
        uint32_t tangentsBaseOffset;
        uint32_t tangentsCount;
        uint32_t texCoords0BaseOffset;
        uint32_t texCoords0Count;

        uint32_t trianglesBaseOffset;
        uint32_t triangleCount;
        uint32_t clusterID;
    };

    struct MeshStorage
    {
        float3 aabbMin;
        float3 aabbMax;
        uint32_t clusterOffset;
        uint32_t clusterCount;
    };

    struct InstanceStorage
    {
        float4x4 localTransform;
        uint32_t meshIdx;
    };

    struct GeometrySource
    {
        SceneViewData scene;
        std::vector<float3> positions;
        std::vector<float3> normals;
        std::vector<float4> tangents;
        std::vector<float2> texCoords0;
        std::vector<uint8_t> indices;
        std::vector<ClusterStorage> clusters;
        std::vector<MeshStorage>  meshes;
        std::vector<InstanceStorage> instances;
    };

    struct MaterialPBRStorage
    {
        float4 baseColor;
        float3 emissive;
        float metallic;
        float roughness;
    };

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
    };
};
#endif // LETTUCE_RENDERING_TYPES_HPP