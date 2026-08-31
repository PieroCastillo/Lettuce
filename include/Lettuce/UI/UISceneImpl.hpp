/*
Created by @PieroCastillo on 2026-08-13
*/
#ifndef LETTUCE_UI_UI_SCENE_IMPL_HPP
#define LETTUCE_UI_UI_SCENE_IMPL_HPP

// standard headers
#include <any>
#include <atomic>
#include <cstdint>
#include <functional>
#include <vector>

// project headers
#include "../Core/api.hpp"
#include "../Foundations/api.hpp"
#include "../Quimera/api.hpp"
#include "./api.hpp"

namespace Lettuce::UI
{
    struct ControlTag {};
    using ControlInstance = Handle<ControlTag>;
    constexpr auto InvalidControl = ControlInstance{ std::numeric_limits<uint32_t>::max(), 0 };

    struct ControlTable
    {
        std::vector<uint32_t> generations;
        std::vector<uint32_t> freeList;

        // control
        std::vector<uint32_t> zIndex;
        std::vector<ControlInstance> parents;
        std::vector<ControlInstance> firstChild;
        std::vector<ControlInstance> nextSibling;

        // layout
        std::vector<float2> size;
        std::vector<VerticalAlignment> vertAligment;
        std::vector<HorizontalAlignment> horAlignment;
        std::vector<float4> margin;
        std::vector<float4> padding;
        std::vector<float4> bounds; // readonly

        // visualization
        std::vector<Lettuce::Quimera::Brush> background;
        std::vector<Lettuce::Quimera::Brush> foreground;
        std::vector<Lettuce::Quimera::Layout> layout;
        std::vector<Lettuce::Quimera::Geometry> geometry;

        // interaction
        std::vector<bool> isEnabled;
        std::vector<bool> isFocused; // readonly

        auto AddControl(ControlInstance parent) -> ControlInstance;
        void RemoveControl(ControlInstance);
    };

    struct UISceneImpl
    {

    };
};
#endif // LETTUCE_UI_UI_SCENE_IMPL_HPP