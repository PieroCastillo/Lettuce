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
    struct ControlInstance
    {
        // control
        uint32_t zIndex;
        uint32_t parent;
        uint32_t firstChild;
        uint32_t prevSibling;
        uint32_t nextSibling;

        // layout
        float2 size;
        VerticalAlignment vertAligment;
        HorizontalAlignment horAlignment;
        float4 margin;
        float4 padding;
        float4 bounds; // readonly

        // visualization
        Lettuce::Quimera::Brush background;
        Lettuce::Quimera::Brush foreground;

        // interaction
        bool isEnabled;
        bool isFocused; // readonly

        /* used for: render data, custom control data, etc*/
        std::any controlData;
    };

    struct UISceneImpl
    {
        
    };
};
#endif // LETTUCE_UI_UI_SCENE_IMPL_HPP