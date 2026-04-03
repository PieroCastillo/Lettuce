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

}

void Compositor::StopAnimations(Visual visual)
{

}

void Compositor::StopAnimations(Material material)
{

}

void Compositor::StopAnimations(Light light)
{

}

void Compositor::PauseAnimations(Visual visual)
{

}

void Compositor::PauseAnimations(Material material)
{

}

void Compositor::PauseAnimations(Light light)
{

}

void Compositor::ResumeAnimations(Visual visual)
{

}

void Compositor::ResumeAnimations(Material material)
{

}

void Compositor::ResumeAnimations(Light light)
{

}

void Compositor::BindImplicitAnimation(Visual visual, AnimatableProperty prop, AnimationToken token)
{

}

void Compositor::BindImplicitAnimation(Material material, AnimatableProperty prop, AnimationToken token)
{

}

void Compositor::BindImplicitAnimation(Geometry geometry, AnimatableProperty prop, AnimationToken token)
{

}

void Compositor::BindImplicitAnimation(Light light, AnimatableProperty prop, AnimationToken token)
{

}

void Compositor::BindImplicitAnimation(DropShadow shadow, AnimatableProperty prop, AnimationToken token)
{

}