// standard headers
#include <algorithm>
#include <array>
#include <memory>
#include <memory_resource>
#include <span>
#include <utility>
#include <vector>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/common.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Quimera/SurfaceImpl.hpp"
#include "Lettuce/Quimera/api.hpp"
#include "Lettuce/Utils/api.hpp"

// external headers
#include "freetype/ftoutln.h"
#include "freetype/ftstroke.h"
#include "harfbuzz/hb-ft.h"

using namespace Lettuce::Quimera;
using namespace Lettuce::Core;

Surface::Surface(const SurfaceDesc& desc)
{
    Create(desc);
}

Surface::~Surface()
{
    Destroy();
}

Surface::Surface(Surface&& other) noexcept : impl(std::exchange(other.impl, nullptr))
{
}

Surface& Surface::operator=(Surface&& other) noexcept
{
    if (this != &other)
    {
        Destroy();
        impl = std::exchange(other.impl, nullptr);
    }
    return *this;
}

void Surface::Create(const SurfaceDesc& desc)
{
    if (impl)
        throw std::logic_error("Surface::Create cannot be called from initizalized Surface.");

    auto nimpl = new SurfaceImpl;

    try
    {
        nimpl->Create(desc);
    }
    catch (...)
    {
        delete nimpl;
        throw;
    }
    impl = nimpl;
}

void Surface::Destroy() noexcept
{
    if (!impl)
        return;

    impl->Destroy();
    delete impl;
    impl = nullptr;
}

auto Surface::CreateAnimation(const NaturalMotionAnimationDesc&) -> Animation
{
    throw NotImplemented("CreateAnimation is not implemented yet.");
}

auto Surface::CreateGeometry(const ImplicitGeometryDesc& desc) -> Geometry
{
    auto geoIdx = impl->bImplicitGeometry.Push({
        desc.leftTopCornerRadious, desc.rightTopCornerRadious,
        desc.leftBottomCornerRadious, desc.rightBottomCornerRadious,
        });

    return impl->geometries.allocate({ geoIdx, (uint32_t)GeometryHeap::Implicit });
}

auto Surface::CreateFont(const FontDesc& desc) -> Font
{
    // ensures font data address
    auto fontData = std::make_unique<uint8_t[]>(desc.fontData.size());
    memcpy(fontData.get(), desc.fontData.data(), desc.fontData.size());

    FT_Face fontFace;
    auto error = FT_New_Memory_Face(impl->fontLib, (FT_Byte*)fontData.get(), desc.fontData.size(), 0, &fontFace);
    DebugAssert(error == FT_Err_Ok, "FreeType Error");

    error = FT_Select_Charmap(fontFace, FT_ENCODING_UNICODE);
    DebugAssert(error == FT_Err_Ok, "FreeType Error");

    std::println("units_per_EM = {}", fontFace->units_per_EM);

    std::println(
        "x_ppem={}, y_ppem={}, x_scale={}, y_scale={}",
        fontFace->size->metrics.x_ppem,
        fontFace->size->metrics.y_ppem,
        fontFace->size->metrics.x_scale,
        fontFace->size->metrics.y_scale
    );

    auto hbFont = hb_ft_font_create_referenced(fontFace);

    return impl->fonts.allocate({ std::move(fontData), fontFace, hbFont });
}

auto Surface::CreateBrush(const SolidColorBrushDesc& desc) -> Brush
{
    auto brushIdx = impl->bSolidColorBrush.Push({ desc.color });
    return impl->brushes.allocate({ brushIdx, (uint32_t)BrushHeap::SolidColor });
}

auto Surface::CreateLayout(const LayoutDesc& desc) -> Layout
{
    auto layoutIdx = impl->bLayouts.Push({ desc.position, desc.scale, desc.skew, desc.anchorPoint, desc.rotation });
    return impl->layouts.allocate({ layoutIdx });
}

void Surface::Destroy(Font fontHandle)
{
    auto& font = impl->fonts.get(fontHandle);
    hb_font_destroy(font.hbFont);
    FT_Done_Face(font.fontFace);
    font.fontData.reset();

    for (auto [_, data] : font.glyphIdxDataMap)
        impl->pDevice->Destroy(data.texture);
}

void Surface::LoadGlyphs(CommandAllocator copyCmdAlloc, Font fontHandle, std::span<const uint32_t> glyphIDs)
{
    auto cmd = impl->pDevice->AllocateCommandBuffer(copyCmdAlloc);
    auto& font = impl->fonts.get(fontHandle);

    auto stagingMems = std::vector<MemoryView>();
    auto texDescriptors = std::vector<std::pair<uint32_t, TextureView>>();

    stagingMems.reserve(glyphIDs.size());
    texDescriptors.reserve(glyphIDs.size());

    // SDF uses uint8 for each pixel
    for (auto glyphId : glyphIDs)
    {
        if (font.glyphIdxDataMap.contains(glyphId))
            continue;

        constexpr auto fontTextureSize = 32u;
        constexpr float invFontSize = 1.0f / fontTextureSize;
        auto error = FT_Set_Pixel_Sizes(font.fontFace, 0, fontTextureSize);
        DebugAssert(error == FT_Err_Ok, "FreeType Error");

        error = FT_Load_Glyph(font.fontFace, glyphId, FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT | FT_LOAD_IGNORE_TRANSFORM | FT_LOAD_NO_BITMAP | FT_LOAD_COMPUTE_METRICS);
        DebugAssert(error == FT_Err_Ok, "FreeType Error");

        auto& glyph = font.fontFace->glyph;

        error = FT_Render_Glyph(glyph, FT_RENDER_MODE_SDF);
        DebugAssert(error == FT_Err_Ok, "FreeType Error");

        auto& dstBitmap = glyph->bitmap;

        if (dstBitmap.rows == 0 || dstBitmap.width == 0)
            continue;

        auto mem = impl->pDevice->CreateMemoryView({ dstBitmap.width * dstBitmap.rows, true });
        auto memAddr = impl->pDevice->GetMemoryViewInfo(mem).cpuAddress;
        memset(memAddr, 0, dstBitmap.width * dstBitmap.rows); // clear memory

        // perform copy from freetype
        for (auto y = 0u; y < dstBitmap.rows; ++y) {
            for (auto x = 0u; x < dstBitmap.width; ++x) {
                uint8_t value = dstBitmap.buffer[y * dstBitmap.pitch + x];
                memAddr[y * dstBitmap.width + x] = value;
            }
        }

        auto tex = impl->pDevice->CreateTextureView(TextureViewDesc{ dstBitmap.width, dstBitmap.rows, 1, Format::Raw_R8_UNorm, 1, 1 });

        // register
        auto descriptorIdx = impl->sampledImgRegistry.Push(tex);
        auto glyphStorageIdx = impl->bGlyphGeometry.Push({ descriptorIdx, glyph->bitmap_top * invFontSize, glyph->bitmap_left * invFontSize });
        font.glyphIdxDataMap[glyphId] = { tex, descriptorIdx, glyphStorageIdx };

        texDescriptors.push_back({ descriptorIdx, tex });

        auto copy = MemoryToTextureCopy{ mem, tex, 0, 0, 0, 1, 0, 0, dstBitmap.width, dstBitmap.rows };

        cmd.MemoryCopy(copy);

        stagingMems.push_back(mem);
    }

    auto pushDescriptors = PushResourceDescriptorsDesc{
        .sampledTextures = std::span(texDescriptors),
        .descriptorTable = impl->dtSurface,
    };
    impl->pDevice->PushResourceDescriptors(pushDescriptors);

    std::array<std::span<CommandBuffer>, 1> cmds = { std::span(&cmd, 1) };

    /* send the copies, wait for them while it's pushing descriptors, and finally erase the old memories */
    auto submit = CommandBufferSubmitDesc{
        .queueType = QueueType::Copy,
        .commandBuffers = std::span(cmds),
    };
    impl->pDevice->Submit(submit);

    // destroy staging mems
    for (auto mem : stagingMems)
        impl->pDevice->Destroy(mem);
}