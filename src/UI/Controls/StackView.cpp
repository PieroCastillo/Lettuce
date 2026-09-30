#include "Lettuce/Core/api.hpp"
#include "Lettuce/Quimera/api.hpp"
#include "Lettuce/UI/types.hpp"
#include "Lettuce/UI/Controls/primitives.hpp"
#include "Lettuce/UI/Controls/controls.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Quimera;
using namespace Lettuce::UI;
using namespace Lettuce::UI::Controls;

auto StackView::Children() const -> std::span<const Primitives::ControlRef>
{
    return children;
}

void StackView::Build(Surface& surf, ControlInstance& instance)
{

}

void StackView::Reset(Surface& surf, ControlInstance& instance)
{

}

auto StackView::Layout(ControlInstance& instance, float4 available) -> float4
{
    return {};
}

void StackView::Update(ControlInstance& instance, const InputState& input)
{

}

void StackView::Render(ControlInstance& instance, SurfaceCommandBuffer& scmd)
{

}