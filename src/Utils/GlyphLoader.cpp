// standard headers
#include <bit>

// project headers
#include "Lettuce/Quimera/SurfaceImpl.hpp"
#include "Lettuce/Utils/GlyphLoader.hpp"

// external headers
#include "harfbuzz/hb.h"

using namespace Lettuce::Core;
using namespace Lettuce::Quimera;
using namespace Lettuce::Utils;

auto GlyphLoader::ShapeText(Surface* surface, Font fontHandle, std::string_view text) -> std::vector<Glyph>
{
    auto& font = surface->GetImplementation()->fonts.get(fontHandle);

    auto* hbBuff = hb_buffer_create();
    // add string view, add all
    hb_buffer_add_utf8(hbBuff, text.data(), text.size(), 0, -1);
    hb_buffer_guess_segment_properties(hbBuff);

    // force max precision
    constexpr auto fontSize = 48.0f;
    constexpr auto fontScale = fontSize * 64.0f;
    constexpr auto invFontScale = 1.0f / fontScale;

    hb_font_set_scale(font.hbFont, fontScale, fontScale);

    auto features = std::vector<hb_feature_t>{
        { hb_tag_from_string("liga", -1), 1, 0, 0xFFFFFFFF },
        { hb_tag_from_string("calt", -1), 1, 0, 0xFFFFFFFF },
    };

    hb_shape(font.hbFont, hbBuff, features.data(), features.size());

    uint32_t glyphCount;
    auto* glyphInfo = hb_buffer_get_glyph_infos(hbBuff, &glyphCount);
    auto* glyphPos = hb_buffer_get_glyph_positions(hbBuff, &glyphCount);
    auto res = std::vector<Glyph>(glyphCount, { .font = fontHandle });

    float cursorX = 0;
    float cursorY = 0;
    for (auto i = 0; i < glyphCount; ++i)
    {
        auto pos = glyphPos[i];
        // save values as raw bits
        res[i] = Glyph{
            .font = fontHandle,
            .offsetX = std::bit_cast<uint32_t>(cursorX + (pos.x_offset * invFontScale)),
            .offsetY = std::bit_cast<uint32_t>(cursorY + (pos.y_offset * invFontScale)),
            .glyphID = glyphInfo[i].codepoint,
        };
        cursorX += pos.x_advance * invFontScale;
        cursorY -= pos.y_advance * invFontScale;
    }

    hb_buffer_destroy(hbBuff);

    return res;
}