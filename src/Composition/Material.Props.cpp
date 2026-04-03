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

void Compositor::SetColor(Material handle, Color value) {}
void Compositor::SetTintColor(Material handle, Color value) {}
void Compositor::SetBlurRadius(Material handle, float value) {}
void Compositor::SetNoiseIntensity(Material handle, float value) {}
void Compositor::SetDistortionStrength(Material handle, float value) {}