#include "Lettuce/Core/api.hpp"
#include "Lettuce/Quimera/api.hpp"
#include "Lettuce/UI/types.hpp"
#include "Lettuce/UI/Controls/primitives.hpp"
#include "Lettuce/UI/Controls/controls.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Quimera;
using namespace Lettuce::UI;
using namespace Lettuce::UI::Controls;

void Menu::Build(Surface&, ControlInstance&)
{

}

void Menu::Reset(Surface&, ControlInstance&)
{

}

auto Menu::Layout(LayoutContext& ctx) -> bool
{
    return {};
}

void Menu::Update(ControlInstance&, const InputState&)
{

}

void Menu::Render(ControlInstance&, SurfaceCommandBuffer&)
{

}

void Menu::Select(uint32_t index)
{

}