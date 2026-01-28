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

void Compositor::Create(const CompositorDesc& desc)
{

}

void Compositor::Destroy()
{

}

void Compositor::BeginBatch()
{

}

void Compositor::EndBatch()
{

}

void Compositor::SetOpacityBatch(std::span<const Visual> visuals, float opacity)
{

}

void Compositor::SetVisibleBatch(std::span<const Visual> visuals, bool visible)
{

}

void Compositor::Commit()
{

}

void Compositor::SetDebugName(Visual visual, const char* name)
{

}

auto Compositor::GetVisualCount() -> uint32_t
{
    return {};
}

auto Compositor::GetAnimationCount() -> uint32_t
{
    return {};
}