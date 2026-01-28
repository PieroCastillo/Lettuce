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

auto Compositor::GetBrushColor(Brush handle) -> Color
{
    return {};
}

auto Compositor::GetBrushSecondaryColor(Brush handle) -> Color
{
    return {};
}

auto Compositor::GetBrushMetallic(Brush handle) -> float
{
    return {};
}

auto Compositor::GetBrushRoughness(Brush handle) -> float
{
    return {};
}

auto Compositor::GetBrushDistortionStrength(Brush handle) -> float
{
    return {};
}

auto Compositor::GetBrushNoiseScale(Brush handle) -> float
{
    return {};
}

void Compositor::SetBrushColor(Brush handle, Color value)
{

}

void Compositor::SetBrushSecondaryColor(Brush handle, Color value)
{

}

void Compositor::SetBrushMetallic(Brush handle, float value)
{
    
}

void Compositor::SetBrushRoughness(Brush handle, float value)
{

}

void Compositor::SetBrushDistortionStrength(Brush handle, float value)
{

}

auto Compositor::GetBrushNoiseIntensity(Brush handle) -> float
{
    return {};
}

void Compositor::SetBrushNoiseScale(Brush handle, float value)
{

}

void Compositor::SetBrushNoiseIntensity(Brush handle, float value)
{

}