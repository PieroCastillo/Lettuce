#include "Lettuce/Core/api.hpp"
#include "Lettuce/Quimera/api.hpp"
#include "Lettuce/UI/types.hpp"
#include "Lettuce/UI/Controls/controls.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Quimera;
using namespace Lettuce::UI;

void Controls::Button::Build(Surface&, ControlInstance&)
{

}

void Controls::Button::Reset(Surface&, ControlInstance&)
{

}

auto Controls::Button::Layout(ControlInstance&, float4 available)->float4
{
    return {};
}

void Controls::Button::Update(ControlInstance&, const InputState&)
{

}

void Controls::Button::Render(ControlInstance&, SurfaceCommandBuffer&)
{

}