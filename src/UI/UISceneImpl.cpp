#include "Lettuce/UI/api.hpp"
#include "Lettuce/UI/UISceneImpl.hpp"

using namespace Lettuce::UI;
using namespace Lettuce::Quimera;

auto ControlTable::AddControl(Control parent) -> Control
{
    uint32_t idx;
    if (!freeList.empty())
    {
        idx = freeList.back();
        freeList.pop_back();

    }
    else
    {
        idx = (uint32_t)parents.size();
        generations.emplace_back(1);

        zIndex.emplace_back();
        parents.emplace_back();
        firstChild.emplace_back();
        nextSibling.emplace_back();

        size.emplace_back();
        vertAligment.emplace_back();
        horAlignment.emplace_back();
        margin.emplace_back();
        padding.emplace_back();

        background.emplace_back();
        foreground.emplace_back();
        layout.emplace_back();
        geometry.emplace_back();

        isEnabled.emplace_back();
        isFocused.emplace_back();
    }

    auto self = Control {idx, generations[idx]};

    zIndex[idx] = 1;
    parents[idx] = parent;
    firstChild[idx] = InvalidControl;
    nextSibling[idx] = {};

    size[idx] = {};
    vertAligment[idx] = {};
    horAlignment[idx] = {};
    margin[idx] = {};
    padding[idx] = {};

    background[idx] = {};
    foreground[idx] = {};
    layout[idx] = {};
    geometry[idx] = {};

    isEnabled[idx] = true;
    isFocused[idx] = false;

    if (parent != InvalidControl)
    {
        nextSibling[idx] = firstChild[parent.index];
        firstChild[parent.index] = self;
    }

    return Control {idx, generations[idx]};
}

void ControlTable::RemoveControl(Control control)
{
    generations[control.index]++;
    freeList.push_back(control.index);

    // remove children
}