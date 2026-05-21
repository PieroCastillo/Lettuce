/*
Created by @PieroCastillo on 2026-05-17
*/
#ifndef LETTUCE_RENDERING_SCENE_TREE_IMPL_HPP
#define LETTUCE_RENDERING_SCENE_TREE_IMPL_HPP

// project headers
#include "../Core/api.hpp"
#include "../Core/ResourcePool.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Rendering
{
    struct ScenePartitionData
    {
        uint64_t scenePartitionAddr;
    };

    struct GeometryClusterData
    {
        uint64_t geometryClusterAddr;
    };

    struct SceneTreeImpl
    {
        Device* device = nullptr;

        DescriptorTable dtSceneTree;

        Pipeline pBuildGeometryCluster;
        Pipeline pBuildScenePartition;
        Pipeline pUpdateScenePartition;

        ResourcePool<ScenePartition, ScenePartitionData> scenePartitions;
        ResourcePool<GeometryCluster, GeometryClusterData> clusterDatas;

        void Create(const SceneTreeDesc& desc);
        void Destroy();
    };
};
#endif // LETTUCE_RENDERING_SCENE_TREE_IMPL_HPP