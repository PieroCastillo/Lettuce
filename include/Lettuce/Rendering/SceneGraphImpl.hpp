/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_RENDERING_SCENE_GRAPH_IMPL_HPP
#define LETTUCE_RENDERING_SCENE_GRAPH_IMPL_HPP

// project headers
#include "api.hpp"
#include "HelperStructs.hpp"
#include "../Core/ResourcePool.hpp"

namespace Lettuce::Rendering
{
    struct SceneGraphImpl
    {
    public:
        Device* device;
        std::shared_ptr<Allocators::IGPUMemoryResource> allocator;

        MemoryView sceneMemoryView;
        MemoryView lightMemoryView;

        static constexpr int maxLightsCount = 256;

        std::span<SceneGPUData> sceneGPUData;
        // std::span<LightGPUData> lightsGPUData;

        // ResourcePool<SceneNode, SceneNodeInfo> nodes;
        // ResourcePool<Renderable, uint64_t> renderables;
        
        // std::vector<SceneNode> nodesVec;

        void Create(const SceneGraphDesc&);
        void Destroy();
    };
}
#endif // LETTUCE_RENDERING_SCENE_GRAPH_IMPL_HPP