/*
Created by @PieroCastillo on 2026-10-25
*/
#ifndef LETTUCE_QUIMERA_TYPES_ACCESS_DATA_HPP
#define LETTUCE_QUIMERA_TYPES_ACCESS_DATA_HPP

// standard headers
#include <memory>
#include <unordered_map>
#include <utility>

// project headers
#include "../Core/basicTypes.hpp"

// external headers
#include "freetype/freetype.h"
#include "harfbuzz/hb.h"

namespace Lettuce::Quimera
{
    struct SurfaceData
    {
        uint32_t width;
        uint32_t height;
        uint32_t drawCommandCount;
        uint32_t layoutCount;
    };

    struct GeometryAccessData
    {
        uint32_t geometryIdx;
        uint32_t geometryHeapIdx;
    };

    struct GlyphAccessData
    {
        TextureView texture;
        uint32_t descriptorIdx;
        uint32_t storageIdx;
    };

    struct FontAccessData
    {
        std::unique_ptr<uint8_t[]> fontData;
        FT_Face fontFace;
        hb_font_t* hbFont;
        // key: glyphID, data: TextureView | DescriptorIdx
        std::unordered_map<uint32_t, GlyphAccessData> glyphIdxDataMap;
    };

    struct BrushAccessData
    {
        uint32_t brushIdx;
        uint32_t brushHeapIdx;
    };

    struct LayoutAccessData
    {
        uint32_t layoutIdx;
    };

    enum class Properties
    {
        Brush_Solid_Color,
        Layout_Position,
        Layout_Scale,
        Layout_Skew,
        Layout_AnchorPoint,
        Layout_Rotation,
        Geometry_Implicit_Width,
        Geometry_Implicit_Height,
        Geometry_Implicit_LeftTop,
        Geometry_Implicit_LeftBottom,
        Geometry_Implicit_RightTop,
        Geometry_Implicit_RightBottom,
    };

    constexpr Properties toProperty(SolidColorBrushProperties property) noexcept
    {
        switch (property)
        {
        case SolidColorBrushProperties::Color:
            return Properties::Brush_Solid_Color;
        }

        std::unreachable();
    }

    constexpr Properties toProperty(LayoutProperties property) noexcept
    {
        switch (property)
        {
        case LayoutProperties::Position:    return Properties::Layout_Position;
        case LayoutProperties::Scale:       return Properties::Layout_Scale;
        case LayoutProperties::Skew:        return Properties::Layout_Skew;
        case LayoutProperties::AnchorPoint: return Properties::Layout_AnchorPoint;
        case LayoutProperties::Rotation:    return Properties::Layout_Rotation;
        }

        std::unreachable();
    }

    constexpr Properties toProperty(ImplicitGeometryProperties property) noexcept
    {
        switch (property)
        {
        case ImplicitGeometryProperties::Width:       return Properties::Geometry_Implicit_Width;
        case ImplicitGeometryProperties::Height:      return Properties::Geometry_Implicit_Height;
        case ImplicitGeometryProperties::LeftTop:     return Properties::Geometry_Implicit_LeftTop;
        case ImplicitGeometryProperties::LeftBottom:  return Properties::Geometry_Implicit_LeftBottom;
        case ImplicitGeometryProperties::RightTop:    return Properties::Geometry_Implicit_RightTop;
        case ImplicitGeometryProperties::RightBottom: return Properties::Geometry_Implicit_RightBottom;
        }

        std::unreachable();
    }

    struct ChangeKey
    {
        uint64_t object;
        Properties property;
        bool operator==(const ChangeKey&) const = default;
    };

    struct ChangeKeyHash {
        size_t operator()(const ChangeKey& k) const noexcept {
            uint64_t h = k.object;

            h ^= uint64_t(k.property) * 0x9E3779B185EBCA87ull;
            h ^= uint64_t(k.object) * 0xC2B2AE3D27D4EB4Full;

            h ^= h >> 30;
            h *= 0xBF58476D1CE4E5B9ull;
            h ^= h >> 27;
            h *= 0x94D049BB133111EBull;
            h ^= h >> 31;

            return static_cast<size_t>(h);
        }
    };
};
#endif // LETTUCE_QUIMERA_TYPES_ACCESS_DATA_HPP