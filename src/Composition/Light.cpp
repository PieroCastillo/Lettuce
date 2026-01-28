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

auto Compositor::CreateLight(const LightDesc& desc) -> Light
{
    return {};
}

void Compositor::DestroyLight(Light light)
{

}

void Compositor::AddLightTarget(Light light, Visual visual)
{

}

void Compositor::RemoveLightTarget(Light light, Visual visual)
{

}