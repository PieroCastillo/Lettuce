#include "Lettuce/Core/api.hpp"
#include "Lettuce/Quimera/api.hpp"
#include "Lettuce/UI/types.hpp"
#include "Lettuce/UI/Controls/controls.hpp"
#include "Lettuce/Utils/api.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Quimera;
using namespace Lettuce::UI;

namespace Lettuce::UI::Controls
{
    struct ButtonData
    {
        std::string text;
        Geometry geometry;
        std::vector<Glyph> glyphs;
        Layout layout;
        Layout layoutTextBase;
        Font font;
    };
}

void Controls::Button::Build(Surface& surf, ControlInstance& instance)
{
    instance.name = std::move(name); 
    instance.size = size;
    instance.vertAligment = verticalAlignment;
    instance.horAlignment = horizontalAlignment;
    instance.margin = margin;
    instance.padding = padding;
    instance.style = std::move(style);

    ButtonData data = {
        .text = std::move(Content),
        // .geometry = surf.CreateGeometry(ImplicitGeometryDesc {}),
        // .glyphs = Lettuce::Utils::GlyphLoader::ShapeText(&surf, , Content), 
        // .layout
        // .layoutTextBase
        // .font
    };

    instance.controlData = std::make_any<ButtonData>(std::move(data));
}

void Controls::Button::Reset(Surface&, ControlInstance&)
{

}

auto Controls::Button::Layout(LayoutContext& ctx) -> bool
{
    return {};
}

void Controls::Button::Update(ControlInstance&, const InputState&)
{

}

void Controls::Button::Render(ControlInstance& instance, SurfaceCommandBuffer& surf)
{
    
}