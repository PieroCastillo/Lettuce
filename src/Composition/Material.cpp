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

auto CreateMaterial(const SolidColorMaterialDesc&) -> Material { return {}; }
auto CreateMaterial(const LinearGradientMaterialDesc&) -> Material { return {}; }
auto CreateMaterial(const RadialGradientMaterialDesc&) -> Material { return {}; }
auto CreateMaterial(const MeshGradientMaterialDesc&) -> Material { return {}; }
auto CreateMaterial(const TextureMaterialDesc&) -> Material { return {}; }
auto CreateMaterial(const GlassMaterialDesc&) -> Material { return {}; }
auto CreateMaterial(const AcrylicMaterialDesc&) -> Material { return {}; }
auto CreateMaterial(const NoiseMaterialDesc&) -> Material { return {}; }
auto CreateMaterial(const DistortionMaterialDesc&) -> Material { return {}; }
void DestroyMaterial(Material Material) {}
void SetMaterial(Visual visual, Material Material) {}
void SetMaterialGradientStops(Material Material, std::span<const GradientStop> stops) {}