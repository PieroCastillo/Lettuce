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

auto Compositor::CreateAnimation(const AnimationDesc& desc) -> AnimationToken
{
    return {};
}

auto Compositor::CreateAnimation(const NaturalMotionAnimationDesc& desc) -> AnimationToken
{
    return {};
}

void Compositor::DestroyAnimation(AnimationToken token)
{
    impl->appQueue.addCommand(OpCode::DestroyAnimation, token.get(), {});
}

void Compositor::StopAnimations(Visual handle)
{
    impl->appQueue.addCommand(OpCode::StopAnimations_Visual, handle.get(), {});
}

void Compositor::StopAnimations(Material handle)
{
    impl->appQueue.addCommand(OpCode::StopAnimations_Material, handle.get(), {});
}

void Compositor::StopAnimations(Light handle)
{
    impl->appQueue.addCommand(OpCode::StopAnimations_Light, handle.get(), {});
}

void Compositor::StopAnimations(Geometry handle)
{
    impl->appQueue.addCommand(OpCode::StopAnimations_Geometry, handle.get(), {});
}

void Compositor::StopAnimations(DropShadow handle)
{
    impl->appQueue.addCommand(OpCode::StopAnimations_DropShadow, handle.get(), {});
}

void Compositor::PauseAnimations(Visual handle)
{
    impl->appQueue.addCommand(OpCode::PauseAnimations_Visual, handle.get(), {});
}

void Compositor::PauseAnimations(Material handle)
{
    impl->appQueue.addCommand(OpCode::PauseAnimations_Material, handle.get(), {});
}

void Compositor::PauseAnimations(Light handle)
{
    impl->appQueue.addCommand(OpCode::PauseAnimations_Light, handle.get(), {});
}

void Compositor::PauseAnimations(Geometry handle)
{
    impl->appQueue.addCommand(OpCode::PauseAnimations_Geometry, handle.get(), {});
}

void Compositor::PauseAnimations(DropShadow handle)
{
    impl->appQueue.addCommand(OpCode::PauseAnimations_DropShadow, handle.get(), {});
}

void Compositor::ResumeAnimations(Visual handle)
{
    impl->appQueue.addCommand(OpCode::ResumeAnimations_Visual, handle.get(), {});
}

void Compositor::ResumeAnimations(Material handle)
{
    impl->appQueue.addCommand(OpCode::ResumeAnimations_Material, handle.get(), {});
}

void Compositor::ResumeAnimations(Light handle)
{
    impl->appQueue.addCommand(OpCode::ResumeAnimations_Light, handle.get(), {});
}

void Compositor::ResumeAnimations(Geometry handle)
{
    impl->appQueue.addCommand(OpCode::ResumeAnimations_Geometry, handle.get(), {});
}

void Compositor::ResumeAnimations(DropShadow handle)
{
    impl->appQueue.addCommand(OpCode::ResumeAnimations_DropShadow, handle.get(), {});
}

void Compositor::BindImplicitAnimation(Visual handle, AnimatableProperty prop, AnimationToken token)
{
    impl->appQueue.addCommand(OpCode::BindImplicitAnimation_Visual, handle.get(), token.get());
}

void Compositor::BindImplicitAnimation(Material handle, AnimatableProperty prop, AnimationToken token)
{
    impl->appQueue.addCommand(OpCode::BindImplicitAnimation_Material, handle.get(), token.get());
}

void Compositor::BindImplicitAnimation(Geometry handle, AnimatableProperty prop, AnimationToken token)
{
    impl->appQueue.addCommand(OpCode::BindImplicitAnimation_Geometry, handle.get(), token.get());
}

void Compositor::BindImplicitAnimation(Light handle, AnimatableProperty prop, AnimationToken token)
{
    impl->appQueue.addCommand(OpCode::BindImplicitAnimation_Light, handle.get(), token.get());
}

void Compositor::BindImplicitAnimation(DropShadow handle, AnimatableProperty prop, AnimationToken token)
{
    impl->appQueue.addCommand(OpCode::BindImplicitAnimation_DropShadow, handle.get(), token.get());
}