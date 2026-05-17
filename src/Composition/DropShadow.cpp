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

auto Compositor::CreateDropShadow(const DropShadowDesc&) -> DropShadow { return {}; }

void Compositor::DestroyDropShadow(DropShadow shadow)
{
    impl->appQueue.addCommand(OpCode::DestroyShadow, shadow.get(), {});
}

void Compositor::SetShadowTarget(Visual visual, DropShadow shadow)
{
    impl->appQueue.addCommand(OpCode::SetShadowTarget, visual.get(), shadow.get(), true);
}

void Compositor::ClearShadowTarget(Visual visual)
{
    impl->appQueue.addCommand(OpCode::ClearShadowTarget, visual.get(), {}, true);
}