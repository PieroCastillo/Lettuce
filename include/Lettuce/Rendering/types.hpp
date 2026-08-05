/*
Created by @PieroCastillo on 2026-05-17
*/
#ifndef LETTUCE_RENDERING_TYPES_HPP
#define LETTUCE_RENDERING_TYPES_HPP

// standard headers
#include <cstdint>
#include <span>

// project headers
#include "../Core/basicTypes.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Rendering
{
    struct SceneViewData
    {
        float4x4 viewProj;
        uint32_t instanceCount;
    };

    struct ClusterStorage
    {
        uint32_t materialHeap;
        uint32_t materialID;

        float4 cone;
        float3 aabbMin;
        float3 aabbMax;
        float3 centroid;

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
};
#endif // LETTUCE_RENDERING_TYPES_HPP