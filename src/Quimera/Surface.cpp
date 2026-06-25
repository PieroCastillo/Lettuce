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
    impl->Destroy();
    delete impl;
}

auto Surface::CreateGeometry(const ImplicitGeometryDesc& desc) -> Geometry
{
    auto geoIdx = impl->bImplicitGeometry.Push({
        desc.bounds.x, desc.bounds.y, desc.bounds.w, desc.bounds.h,
        desc.leftTopCornerRadious, desc.rightTopCornerRadious,
        desc.leftBottomCornerRadious, desc.rightBottomCornerRadious,
        });

    return impl->geometries.allocate({ geoIdx, (uint32_t)GeometryHeap::Implicit});
}

auto Surface::CreateBrush(const SolidColorBrushDesc& desc) -> Brush
{
    auto brushIdx = impl->bSolidColorBrush.Push({ desc.color });
    return impl->brushes.allocate({ brushIdx, (uint32_t)BrushHeap::SolidColor });
}