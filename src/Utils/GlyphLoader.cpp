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

    constexpr float units = 32.0f;
    constexpr float invUnit = 1.0f / units;
    hb_font_set_scale(font.hbFont, units, units);
    hb_shape(font.hbFont, hbBuff, 0, 0);

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
            .offsetX = std::bit_cast<uint32_t>(cursorX + (pos.x_offset * invUnit)),
            .offsetY = std::bit_cast<uint32_t>(cursorY + (pos.y_offset * invUnit)),
            .glyphID = glyphInfo[i].codepoint,
        };
        cursorX += pos.x_advance*invUnit;
        cursorY -= pos.y_advance*invUnit;
    }

    hb_buffer_destroy(hbBuff);

    return res;
}