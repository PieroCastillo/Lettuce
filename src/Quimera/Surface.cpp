// standard headers
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
    std::ranges::copy(desc.fontData, fontData.get());

    FT_Face fontFace;
    auto error = FT_New_Memory_Face(impl->fontLib, (FT_Byte*)fontData.get(), desc.fontData.size(), 0, &fontFace);
    error = FT_Set_Pixel_Sizes(fontFace, 32, 32);
    DebugAssert(error == FT_Err_Ok, "FreeType Error");

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
}

void Surface::LoadGlyphs(CommandAllocator copyCmdAlloc, Font fontHandle, std::span<const uint32_t> glyphIDs)
{
    auto cmd = impl->pDevice->AllocateCommandBuffer(copyCmdAlloc);
    auto& font = impl->fonts.get(fontHandle);

    // auto texs = std::vector<TextureView>();
    auto stagingMems = std::vector<MemoryView>();
    auto texDescriptors = std::vector<std::pair<uint32_t, TextureView>>();
    // texs.reserve(glyphIDs.size());
    stagingMems.reserve(glyphIDs.size());
    texDescriptors.reserve(glyphIDs.size());

    // SDF uses uint8 for each pixel
    for (auto glyphId : glyphIDs)
    {
        if (font.glyphIdxDataMap.contains(glyphId))
            continue;

        FT_Load_Glyph(font.fontFace, glyphId, FT_LOAD_DEFAULT);
        FT_Render_Glyph(font.fontFace->glyph, FT_RENDER_MODE_SDF);

        auto mem = impl->pDevice->CreateMemoryView({ 32 * 32, true });
        auto memAddr = impl->pDevice->GetMemoryViewInfo(mem).cpuAddress;
        memcpy(memAddr, font.fontFace->glyph->bitmap.buffer, 32 * 32);

        auto tex = impl->pDevice->CreateTextureView(TextureViewDesc{ 32, 32, 1, Format::Raw_R8_UNorm, 1, 1 });

        // register
        auto idx = impl->sampledImgRegistry.Push(tex);
        font.glyphIdxDataMap[glyphId] = std::make_pair(tex, idx);
        texDescriptors.push_back({ idx, tex });

        auto copy = MemoryToTextureCopy{ mem, tex, 0, 0, 0, 1, 0, 0, 32, 32 };

        cmd.MemoryCopy(copy);

        // texs.push_back(tex);
        stagingMems.push_back(mem);
    }

    std::array<std::span<CommandBuffer>, 1> cmds = { std::span(&cmd, 1) };

    /* send the copies, wait for them while it's pushing descriptors, and finally erase the old memories */
    auto submit = CommandBufferSubmitDesc{
        .queueType = QueueType::Copy,
        .commandBuffers = std::span(cmds),
    };
    auto wait = impl->pDevice->SubmitAsync(submit);

    auto pushDescriptors = PushResourceDescriptorsDesc{
        .sampledTextures = std::span(texDescriptors),
        .descriptorTable = impl->dtSurface,
    };
    impl->pDevice->PushResourceDescriptors(pushDescriptors);

    impl->pDevice->WaitFor(wait);
    for (auto mem : stagingMems)
        impl->pDevice->Destroy(mem);
}