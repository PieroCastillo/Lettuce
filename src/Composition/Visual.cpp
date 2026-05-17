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

auto Compositor::CreateVisual(const ContainerVisualDesc&) -> Visual { return {}; }
auto Compositor::CreateVisual(const SpriteVisualDesc&) -> Visual { return {}; }

void Compositor::DestroyVisual(Visual visual)
{
    impl->appQueue.addCommand(OpCode::DestroyVisual, visual.get(), {});
}

void Compositor::AddChild(Visual parent, Visual child)
{
    impl->appQueue.addCommand(OpCode::AddChild, parent.get(), child.get());
}

void Compositor::RemoveChild(Visual parent, Visual child)
{
    impl->appQueue.addCommand(OpCode::RemoveChild, parent.get(), child.get());
}

void Compositor::SetVisible(Visual visual, bool visible)
{
    impl->appQueue.addCommand(OpCode::SetVisible, visual.get(), {}, visible);
}

void Compositor::SetBlendMode(Visual visual, BlendMode mode)
{
    impl->appQueue.addCommand(OpCode::SetBlendMode, visual.get(), {}, mode);
}

void Compositor::SetRectClip(Visual visual, Rect rect)
{
    impl->appQueue.addCommand(OpCode::SetRectClip, visual.get(), {}, std::array<float, 4>{rect.x, rect.y, rect.width, rect.height});
}

void Compositor::SetRoundedRectClip(Visual visual, Rect rect, float cornerRadius)
{
    impl->appQueue.addCommand(OpCode::SetRoundedRectClip, visual.get(), {}, std::array<float, 4>{rect.width, rect.height, rect.x, rect.y});
}

void Compositor::ClearClip(Visual visual)
{
    impl->appQueue.addCommand(OpCode::ClearClip, visual.get(), {});
}

auto Compositor::IsVisible(Visual visual) -> bool { return false; }

auto Compositor::HitTest(Vec2 screenPoint) -> Visual { return {}; }