#include "Lettuce/Core/api.hpp"
#include "Lettuce/Quimera/api.hpp"
#include "Lettuce/UI/types.hpp"
#include "Lettuce/UI/Controls/controls.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Quimera;
using namespace Lettuce::UI;

auto Controls::Label::Build(Surface&) -> ControlInstance
{
    ControlInstance control = {};

    return control;
}

void Controls::Label::Reset(Surface&, ControlInstance&)
{

}

auto Controls::Label::Layout(ControlInstance&, float4 available)->float4
{
    return {};
}

void Controls::Label::Update(ControlInstance&, const InputState&)
{

}

void Controls::Label::Render(ControlInstance&, SurfaceCommandBuffer&)
{

}