/*
Created by @PieroCastillo on 2026-10-25
*/
#ifndef LETTUCE_QUIMERA_TYPES_STORAGE_DATA_HPP
#define LETTUCE_QUIMERA_TYPES_STORAGE_DATA_HPP

// project headers
#include "../Core/basicTypes.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Quimera
{
    struct ImplicitGeometryStorage
    {
        float ctl, ctr, cbl, cbr; // corners
    };

    struct GlyphGeometryStorage
    {
        uint32_t descriptorIdx;
        // normalized values
        float bitmapTop;
        float bitmapLeft;
        float bitmapWidth;
        float bitmapHeight;
    };

    struct SolidColorBrushStorage
    {
        float4 color;
    };

    struct LayoutStorage
    {
        float2 position;
        float2 scale;
        float2 skew;
        float2 anchorPoint;
        float rotation;
        float reserved1;
    };
};
#endif // LETTUCE_QUIMERA_TYPES_STORAGE_DATA_HPP