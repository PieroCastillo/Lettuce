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

auto Compositor::GetColor(Light handle) -> Color { return {}; }
auto Compositor::GetIntensity(Light handle) -> float { return {}; }
auto Compositor::GetPosition(Light handle) -> Vec3 { return {}; }
auto Compositor::GetDirection(Light handle) -> Vec3 { return {}; }
auto Compositor::GetRadius(Light handle) -> float { return {}; }

void Compositor::SetColor(Light handle, Color value)
{
    impl->appQueue.addCommand(OpCode::SetLightColor, handle.get(), {}, std::array<float, 4>{value.r, value.g, value.b, value.a});
}

void Compositor::SetIntensity(Light handle, float value)
{
    impl->appQueue.addCommand(OpCode::SetLightIntensity, handle.get(), {}, std::array<float, 4>{value});
}

void Compositor::SetPosition(Light handle, Vec3 value)
{
    impl->appQueue.addCommand(OpCode::SetLightPosition, handle.get(), {}, std::array<float, 4>{value.x, value.y, value.z});
}

void Compositor::SetDirection(Light handle, Vec3 value)
{
    impl->appQueue.addCommand(OpCode::SetLightDirection, handle.get(), {}, std::array<float, 4>{value.x, value.y, value.z});
}

void Compositor::SetRadius(Light handle, float value)
{
    impl->appQueue.addCommand(OpCode::SetLightRadius, handle.get(), {}, std::array<float, 4>{value});
}