// standard headers
#include <algorithm>
#include <array>
#include <memory>
#include <span>
#include <utility>
#include <vector>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/common.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Quimera/SurfaceImpl.hpp"
#include "Lettuce/Quimera/api.hpp"

using namespace Lettuce::Quimera;
using namespace Lettuce::Core;

void Surface::SetChange(Layout obj, LayoutProperties prop, float4 target, std::optional<Animation> animation)
{
    std::scoped_lock lock(impl->m_animationMutex);

    auto key = ChangeKey{ static_cast<uint64_t>(obj), toProperty(prop) };
    impl->m_animationChanges.insert_or_assign(key, target);
}

void Surface::SetChange(Geometry obj, ImplicitGeometryProperties prop, float4 target, std::optional<Animation> animation)
{
    std::scoped_lock lock(impl->m_animationMutex);

    if (impl->geometries.get(obj).geometryHeapIdx != (uint32_t)GeometryHeap::Implicit) [[unlikely]]
        throw std::logic_error("obj IS NOT an Implicit Geometry");

    auto key = ChangeKey{ static_cast<uint64_t>(obj), toProperty(prop) };
    impl->m_animationChanges.insert_or_assign(key, target);
}

void Surface::SetChange(Brush obj, SolidColorBrushProperties prop, float4 target, std::optional<Animation> animation)
{
    std::scoped_lock lock(impl->m_animationMutex);

    if (impl->brushes.get(obj).brushHeapIdx != (uint32_t)BrushHeap::SolidColor) [[unlikely]]
        throw std::logic_error("obj IS NOT an Solid Color Brush");

    auto key = ChangeKey{ static_cast<uint64_t>(obj), toProperty(prop) };
    impl->m_animationChanges.insert_or_assign(key, target);
}