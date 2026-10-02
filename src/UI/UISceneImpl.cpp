#include "Lettuce/UI/api.hpp"
#include "Lettuce/UI/UISceneImpl.hpp"

using namespace Lettuce::UI;
using namespace Lettuce::Quimera;

void UISceneImpl::Create(const UISceneDesc& desc)
{
    m_surface = &desc.surface;
    m_defaultStyle = desc.defaultStyle;
}

void UISceneImpl::Destroy()
{
    m_allocator.release();
    for (auto& instance : m_instances)
        if (instance.reset)
            instance.reset(*m_surface, instance);
    m_instances.clear();
}