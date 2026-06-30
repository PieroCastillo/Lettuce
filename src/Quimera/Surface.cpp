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
        desc.size.w, desc.size.h,
        desc.leftTopCornerRadious, desc.rightTopCornerRadious,
        desc.leftBottomCornerRadious, desc.rightBottomCornerRadious,
        });

    return impl->geometries.allocate({ geoIdx, (uint32_t)GeometryHeap::Implicit });
}

auto Surface::CreateBrush(const SolidColorBrushDesc& desc) -> Brush
{
    auto brushIdx = impl->bSolidColorBrush.Push({ desc.color });
    return impl->brushes.allocate({ brushIdx, (uint32_t)BrushHeap::SolidColor });
}

auto Surface::CreateLayout(const LayoutDesc& desc) -> Layout
{
    auto layoutIdx = impl->bLayouts.Push({ desc.position, desc.scale, desc.skew, desc.anchorPoint, desc.rotation });
    return impl->layouts.allocate({ layoutIdx });
}