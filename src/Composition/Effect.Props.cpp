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

auto Compositor::GetEffectIntensity(Effect handle) -> float
{
    return {};
}

auto Compositor::GetEffectColor(Effect handle) -> Color
{
    return {};
}

auto Compositor::GetEffectOffset(Effect handle) -> Vec2
{
    return {};
}

auto Compositor::GetEffectRadius(Effect handle) -> float
{
    return {};
}

auto Compositor::GetEffectBlurAmount(Effect handle) -> float
{
    return {};
}

auto Compositor::GetEffectTintOpacity(Effect handle) -> float
{
    return {};
}

auto Compositor::GetEffectTintColor(Effect handle) -> Color
{
    return {};
}

void Compositor::SetEffectIntensity(Effect handle, float value)
{

}

void Compositor::SetEffectColor(Effect handle, Color value)
{

}

void Compositor::SetEffectOffset(Effect handle, Vec2 value)
{

}

void Compositor::SetEffectRadius(Effect handle, float value)
{

}

void Compositor::SetEffectBlurAmount(Effect handle, float value)
{

}

void Compositor::SetEffectTintOpacity(Effect handle, float value)
{

}

void Compositor::SetEffectTintColor(Effect handle, Color value)
{

}