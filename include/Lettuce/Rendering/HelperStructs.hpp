/*
Created by @PieroCastillo on 2026-02-02
*/
#ifndef LETTUCE_RENDERING_HELPER_STRUCTS_HPP
#define LETTUCE_RENDERING_HELPER_STRUCTS_HPP

// standard headers
#include <vector>

// project headers
#include "api.hpp"

namespace Lettuce::Rendering
{
    struct SceneNodeInfo
    {
        std::shared_ptr<IMaterial> material;

        MemoryView materialDataView;
        MemoryView geometryDataView;
        MemoryView vertexBuffer;
        MemoryView indexBuffer;

        std::span<std::any> materialsGPUData;
        std::span<std::any> vertexGPUData;
        std::span<uint32_t> indexGPUData;

        IndirectSet indirectSet;
    };
};
#endif // LETTUCE_RENDERING_HELPER_STRUCTS_HPP