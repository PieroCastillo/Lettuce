/*
Created by @PieroCastillo on 2026-05-30
*/
#ifndef LETTUCE_UTILS_ASSET_LOADER_HPP
#define LETTUCE_UTILS_ASSET_LOADER_HPP

#include "../Core/api.hpp"
#include "../Rendering/api.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Rendering;

namespace Lettuce::Utils
{
    struct MeshL
    {
        float3 aabbMin;
        float3 aabbMax;
        uint32_t clusterOffset;
        uint32_t clusterCount;
    };

    struct VertexL
    {
        float3 pos;
        float3 norm;
        float2 texCoord0;
    };

    struct Instance
    {
        float4x4 transform;
        uint32_t meshIdx;
    };

    struct ModelGeometry
    {
        std::vector<VertexL> vertexTable;
        std::vector<uint8_t> trianglesTable;
        std::vector<ClusterBuildIndirectCommand> clusterBuilds;
        std::vector<MeshL> meshes;
    };

    struct AssetLoader
    {
        static auto LoadSpirv(Device* pDevice, std::string_view path) -> ShaderBinary;
        static auto LoadKtx2Texture(Device* pDevice, CommandAllocator commandAllocator, std::string_view path, uint32_t levels = 1, bool highQuality = false) -> TextureView;
        static auto LoadGltfModel(Device* pDevice, std::string_view path) -> ModelGeometry;
    };
};
#endif // LETTUCE_UTILS_ASSET_LOADER_HPP