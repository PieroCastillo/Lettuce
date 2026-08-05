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
    struct AssetLoader
    {
        static auto LoadSpirv(Device* pDevice, std::string_view path) -> ShaderBinary;
        static auto LoadKtx2Texture(Device* pDevice, CommandAllocator commandAllocator, std::string_view path, uint32_t levels = 1, bool highQuality = false) -> TextureView;
        static auto LoadGtlfAsGeometry(Device* pDevice, std::string_view path) -> GeometrySource;
    };
};
#endif // LETTUCE_UTILS_ASSET_LOADER_HPP