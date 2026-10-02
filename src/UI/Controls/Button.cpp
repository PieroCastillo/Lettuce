#include "Lettuce/Core/api.hpp"
#include "Lettuce/Quimera/api.hpp"
#include "Lettuce/UI/types.hpp"
#include "Lettuce/UI/Controls/controls.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Quimera;
using namespace Lettuce::UI;

namespace Lettuce::UI::Controls
{
    struct ButtonData
    {
        Layout rectLayout;
        std::string text;
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