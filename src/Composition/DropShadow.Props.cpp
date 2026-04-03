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

auto Compositor::GetColor(DropShadow handle) -> Color { return {}; }
auto Compositor::GetBlurRadius(DropShadow handle) -> float { return {}; }
auto Compositor::GetOffset(DropShadow handle) -> Vec2 { return {}; }
auto Compositor::GetOpacity(DropShadow handle) -> float { return {}; }

void Compositor::SetColor(DropShadow handle, Color value) {}
void Compositor::SetBlurRadius(DropShadow handle, float value) {}
void Compositor::SetOffset(DropShadow handle, Vec2 value) {}
void Compositor::SetOpacity(DropShadow handle, float value) {}