#include "Lettuce/UI/api.hpp"
#include "Lettuce/UI/UISceneImpl.hpp"

using namespace Lettuce::UI;
using namespace Lettuce::Quimera;

void UISceneImpl::Create(const UISceneDesc& desc)
{
    m_surface = &desc.surface;
}

void UISceneImpl::Destroy()
{

}