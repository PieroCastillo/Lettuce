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

auto Compositor::CreateNaturalMotionAnimation(const NaturalMotionAnimationDesc& desc) -> AnimationToken
{
    return {};
}

void Compositor::DestroyAnimation(AnimationToken token)
{

}

void Compositor::StopAnimations(Visual visual)
{

}

void Compositor::StopAnimations(Brush brush)
{

}

void Compositor::StopAnimations(Light light)
{

}

void Compositor::StopAnimations(Effect effect)
{

}

void Compositor::StopAnimation(AnimationToken animation)
{

}


void Compositor::PauseAnimations(Visual visual)
{

}

void Compositor::PauseAnimations(Brush brush)
{

}

void Compositor::PauseAnimations(Light light)
{

}

void Compositor::PauseAnimations(Effect effect)
{

}

void Compositor::PauseAnimation(AnimationToken animation)
{

}


void Compositor::ResumeAnimations(Visual visual)
{

}

void Compositor::ResumeAnimations(Brush brush)
{

}

void Compositor::ResumeAnimations(Light light)
{

}

void Compositor::ResumeAnimations(Effect effect)
{

}

void Compositor::ResumeAnimation(AnimationToken animation)
{

}