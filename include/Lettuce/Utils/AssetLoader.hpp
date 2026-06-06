/*
Created by @PieroCastillo on 2026-05-30
*/
#ifndef LETTUCE_UTILS_ASSET_LOADER_HPP
#define LETTUCE_UTILS_ASSET_LOADER_HPP

#include "../Core/api.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Utils
{
    struct PrimitiveInfo
    {
        uint32_t vertexOffset;
        uint32_t vertexCount;
        uint32_t indexOffset;
        uint32_t indexCount;
    };

    struct MeshInfo
    {
        uint32_t primitiveOffset;
        uint32_t primitiveCount;
    };

    struct MeshT
    {
        float3 aabbMin;
        float4 aabbMax;
        uint32_t clusterOffset;
        uint32_t clusterCount;
    };

    class ModelGeometry
    {
    public:
        MemoryView mvPositions;
        MemoryView mvIndices;

        std::vector<PrimitiveInfo> primitives;
        std::vector<MeshInfo> meshes;
    };

    struct AssetLoader
    {
        static auto LoadSpirv(Device* pDevice, std::string_view path) -> ShaderBinary;
        static auto LoadKtx2Texture(Device* pDevice, CommandAllocator commandAllocator, std::string_view path, uint32_t levels = 1, bool highQuality = false) -> TextureView;
        static auto LoadGltfModel(Device* pDevice, std::string_view path) -> ModelGeometry;
    };
};
#endif // LETTUCE_UTILS_ASSET_LOADER_HPP