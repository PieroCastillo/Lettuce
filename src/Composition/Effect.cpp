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

auto Compositor::CreateEffect(const EffectDesc& desc) -> Effect
{
    return {};
}

void Compositor::DestroyEffect(Effect effect)
{

}

void Compositor::AddEffect(Visual visual, Effect effect)
{

}

void Compositor::RemoveEffect(Visual visual, Effect effect)
{

}

void Compositor::ClearEffects(Visual visual)
{

}