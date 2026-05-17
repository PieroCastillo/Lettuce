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

auto Compositor::CreateGeometry(const ImplicitGeometryDesc&) -> Geometry { return {}; }
auto Compositor::CreateGeometry(const InstancedGeometryDesc&) -> Geometry { return {}; }
auto Compositor::CreateGeometry(const PathGeometryDesc&) -> Geometry { return {}; }

void Compositor::DestroyGeometry(Geometry geom)
{
        impl->appQueue.addCommand(OpCode::DestroyGeometry, geom.get(), {});
}

void Compositor::UpdateInstancedGeometry(Geometry geom, std::span<const AtlasInstance> instances)
{
    impl->appQueue.addCommand(OpCode::UpdateInstancedGeometry, geom.get(), {}, instances);
}

void Compositor::UpdatePathGeometry(Geometry geom, std::span<const PathCommand> commands)
{
    impl->appQueue.addCommand(OpCode::UpdatePathGeometry, geom.get(), {}, commands);
}