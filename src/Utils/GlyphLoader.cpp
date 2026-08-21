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
    hb_buffer_guess_segment_properties(hbBuff);

    hb_shape(font.hbFont, hbBuff, 0, 0);

    uint32_t glyphCount;
    auto* glyphInfo = hb_buffer_get_glyph_infos(hbBuff, &glyphCount);
    auto* glyphPos = hb_buffer_get_glyph_positions(hbBuff, &glyphCount);
    auto res = std::vector<Glyph>(glyphCount, { .font = fontHandle });

    auto cursorX = 0u;
    auto cursorY = 0u;
    for (auto i = 0; i < glyphCount; ++i)
    {
        auto pos = glyphPos[i];
        res[i] = Glyph{
            .offsetX = cursorX + pos.x_offset,
            .offsetY = cursorY + pos.y_offset,
            .glyphID = glyphInfo[i].codepoint,
        };
        cursorX += pos.x_advance;
        cursorY += pos.y_advance;
    }
    hb_buffer_destroy(hbBuff);

    return res;
}