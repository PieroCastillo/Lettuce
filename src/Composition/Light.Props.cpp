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

auto Compositor::GetLightColor(Light handle) -> Color
{
    return {};
}

auto Compositor::GetLightIntensity(Light handle) -> float
{
    return {};
}

auto Compositor::GetLightPosition(Light handle) -> Vec3
{
    return {};
}

auto Compositor::GetLightDirection(Light handle) -> Vec3
{
    return {};
}

void Compositor::SetLightColor(Light handle, Color value)
{

}

void Compositor::SetLightIntensity(Light handle, float value)
{

}

void Compositor::SetLightPosition(Light handle, Vec3 value)
{

}

void Compositor::SetLightDirection(Light handle, Vec3 value)
{

}