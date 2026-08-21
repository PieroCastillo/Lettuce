/*
Created by @PieroCastillo on 2026-08-20
*/
#ifndef LETTUCE_UTILS_ASSET_LOADER_HPP
#define LETTUCE_UTILS_ASSET_LOADER_HPP

// standard headers
#include <cstdint>
#include <string_view>
#include <vector>

// project headers
#include "../Core/api.hpp"
#include "../Quimera/api.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Quimera;

namespace Lettuce::Utils
{
    struct GlyphLoader
    {
        static auto ShapeText(Surface* surface, Font font, std::string_view text) -> std::vector<Glyph>;
    };
};
#endif // LETTUCE_UTILS_ASSET_LOADER_HPP