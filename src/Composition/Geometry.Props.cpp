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

auto Compositor::GetCornerRadius(Geometry handle) -> float { return {}; }
auto Compositor::GetStrokeWidth(Geometry handle) -> float { return {}; }
auto Compositor::GetTrimStart(Geometry handle) -> float { return {}; }
auto Compositor::GetTrimEnd(Geometry handle) -> float { return {}; }
auto Compositor::GetTrimOffset(Geometry handle) -> float { return {}; }

void Compositor::SetCornerRadius(Geometry handle, float value) {}
void Compositor::SetStrokeWidth(Geometry handle, float value) {}
void Compositor::SetTrimStart(Geometry handle, float value) {}
void Compositor::SetTrimEnd(Geometry handle, float value) {}
void Compositor::SetTrimOffset(Geometry handle, float value) {}