// standard headers
#include <memory>
#include <memory_resource>
#include <span>
#include <vector>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/common.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Quimera/SurfaceImpl.hpp"
#include "Lettuce/Quimera/api.hpp"
#include "Lettuce/Utils/api.hpp"

using namespace Lettuce::Quimera;
using namespace Lettuce::Core;

Surface::Surface(const SurfaceDesc& desc)
{
    Create(desc);
}

Surface::~Surface()
{
    Destroy();
}

Surface::Surface(Surface&& other) noexcept : impl(std::exchange(other.impl, nullptr))
{
}

Surface& Surface::operator=(Surface&& other) noexcept
{
    if (this != &other)
    {
        Destroy();
        impl = std::exchange(other.impl, nullptr);
    }
    return *this;
}

void Surface::Create(const SurfaceDesc& desc)
{
    if (impl)
        throw std::logic_error("Surface::Create cannot be called from initizalized Surface.");

    auto nimpl = new SurfaceImpl;

    try
    {
        nimpl->Create(desc);
    }
    catch (...)
    {
        delete nimpl;
        throw;
    }
    impl = nimpl;
}

void Surface::Destroy() noexcept
{
    if (!impl)
        return;

    impl->Destroy();
    delete impl;
    impl = nullptr;
}

auto Surface::CreateAnimation(const NaturalMotionAnimationDesc&) -> Animation
{
    throw NotImplemented("CreateAnimation is not implemented yet.");
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