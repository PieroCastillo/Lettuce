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

auto Compositor::GetColor(Material handle) -> Color { return {}; }
auto Compositor::GetTintColor(Material handle) -> Color { return {}; }
auto Compositor::GetBlurRadius(Material handle) -> float { return {}; }
auto Compositor::GetNoiseIntensity(Material handle) -> float { return {}; }
auto Compositor::GetDistortionStrength(Material handle) -> float { return {}; }

void Compositor::SetColor(Material handle, Color value)
{
    impl->appQueue.addCommand(OpCode::SetMaterialColor, handle.get(), {}, std::array<float, 4>{value.r, value.g, value.b, value.a});
}

void Compositor::SetTintColor(Material handle, Color value)
{
    impl->appQueue.addCommand(OpCode::SetMaterialTintColor, handle.get(), {}, std::array<float, 4>{value.r, value.g, value.b, value.a});
}

void Compositor::SetBlurRadius(Material handle, float value)
{
    impl->appQueue.addCommand(OpCode::SetMaterialBlurRadius, handle.get(), {}, std::array<float, 4>{value});
}

void Compositor::SetNoiseIntensity(Material handle, float value)
{
    impl->appQueue.addCommand(OpCode::SetMaterialNoiseIntensity, handle.get(), {}, std::array<float, 4>{value});
}

void Compositor::SetDistortionStrength(Material handle, float value)
{
    impl->appQueue.addCommand(OpCode::SetMaterialDistortionStrength, handle.get(), {}, std::array<float, 4>{value});
}