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

auto Compositor::CreateLight(const AmbientLightDesc&) -> Light { return {}; }
auto Compositor::CreateLight(const DirectionalLightDesc&) -> Light { return {}; }
auto Compositor::CreateLight(const SpotLightDesc&) -> Light { return {}; }
auto Compositor::CreateLight(const RevealLightDesc&) -> Light { return {}; }

void Compositor::DestroyLight(Light light)
{
    impl->appQueue.addCommand(OpCode::DestroyLight, light.get(), {});
}

void Compositor::AddLightTarget(Visual visual, Light light)
{
    impl->appQueue.addCommand(OpCode::AddLightTarget, visual.get(), light.get());
}

void Compositor::RemoveLightTarget(Visual visual)
{
    impl->appQueue.addCommand(OpCode::RemoveLightTarget, visual.get(), {});
}