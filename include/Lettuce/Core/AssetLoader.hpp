/*
Created by @PieroCastillo on 2025-10-23
*/
#ifndef ASSET_LOADER
#define ASSET_LOADER

// standard headers
#include <string>

// project headers
#include "common.hpp"

namespace Lettuce::Core
{
    struct TextureData
    {

    };

    class AssetLoader
    {
    public:
        TextureData loadTexture(std::string name);
    };
};

#endif // ASSET_LOADER