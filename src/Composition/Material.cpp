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

auto Compositor::CreateMaterial(const SolidColorMaterialDesc&) -> Material { return {}; }
auto Compositor::CreateMaterial(const LinearGradientMaterialDesc&) -> Material { return {}; }
auto Compositor::CreateMaterial(const RadialGradientMaterialDesc&) -> Material { return {}; }
auto Compositor::CreateMaterial(const MeshGradientMaterialDesc&) -> Material { return {}; }
auto Compositor::CreateMaterial(const TextureMaterialDesc&) -> Material { return {}; }
auto Compositor::CreateMaterial(const GlassMaterialDesc&) -> Material { return {}; }
auto Compositor::CreateMaterial(const AcrylicMaterialDesc&) -> Material { return {}; }
auto Compositor::CreateMaterial(const NoiseMaterialDesc&) -> Material { return {}; }
auto Compositor::CreateMaterial(const DistortionMaterialDesc&) -> Material { return {}; }

void Compositor::DestroyMaterial(Material material)
{
    impl->appQueue.addCommand(OpCode::DestroyMaterial, material.get(), {});
}

void Compositor::SetMaterial(Visual visual, Material material)
{
    impl->appQueue.addCommand(OpCode::SetMaterial, visual.get(), material.get());
}

void Compositor::SetMaterialGradientStops(Material material, std::span<const GradientStop> stops)
{
    impl->appQueue.addCommand(OpCode::SetMaterialGradientStops, material.get(), {}, stops);
}