// standard headers
#include <memory>
#include <memory_resource>
#include <span>
#include <vector>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Quimera/SurfaceImpl.hpp"
#include "Lettuce/Quimera/api.hpp"

using namespace Lettuce::Quimera;
using namespace Lettuce::Core;

void Surface::Create(const SurfaceDesc& desc)
{
    impl = new SurfaceImpl;
    impl->Create(desc);
}

void Surface::Destroy()
{
    delete impl;
}

auto Surface::CreateGeometry(const ImplicitGeometryDesc& desc) -> Geometry
{

}

auto Surface::CreateBrush(const SolidColorBrushDesc& desc) -> Brush
{
    
}

void Surface::Draw(Geometry geometry, Brush brush, float3x3 transform)
{

}