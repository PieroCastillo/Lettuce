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

void Compositor::SetColor(Light handle, Color value) {}
void Compositor::SetIntensity(Light handle, float value) {}
void Compositor::SetPosition(Light handle, Vec3 value) {}
void Compositor::SetDirection(Light handle, Vec3 value) {}
void Compositor::SetRadius(Light handle, float value) {}