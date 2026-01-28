// standard headers
#include <algorithm>
#include <atomic>
#include <limits>
#include <queue>
#include <string>
#include <vector>

// project headers
#include "Lettuce/Composition/api.hpp"
#include "Lettuce/Composition/CompositorImpl.hpp"
#include "Lettuce/Composition/HelperStructs.hpp"

using namespace Lettuce::Composition;

auto Compositor::CreateVisual() -> Visual
{
    return {};
}

void Compositor::DestroyVisual(Visual visual)
{

}

void Compositor::SetVisible(Visual visual, bool visible)
{

}

auto Compositor::IsVisible(Visual visual) -> bool
{
    return {};
}

void Compositor::SetBlendMode(Visual visual, BlendMode mode)
{

}

void Compositor::SetRectClip(Visual visual, Rect rect)
{

}

void Compositor::SetRoundedRectClip(Visual visual, Rect rect, float cornerRadius)
{

}

void Compositor::ClearClip(Visual visual)
{

}