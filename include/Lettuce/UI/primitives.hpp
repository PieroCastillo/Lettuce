/*
Created by @PieroCastillo on 2026-08-29
*/
#ifndef LETTUCE_UI_PRIMITIVES_HPP
#define LETTUCE_UI_PRIMITIVES_HPP

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
#include "mvvm.hpp"

namespace Lettuce::UI
{
    struct Control
    {
        std::string name;
        Style style;
        VerticalAlignment verticalAlignment;
        HorizontalAlignment horizontalAlignment;
    };

    struct ItemControl : Control
    {
        ObservableVector<std::any> Items;
        std::move_only_function<Control(std::any)> ItemTemplate;
    };

    struct ViewControl : Control
    {
        std::vector<Control> Children;
    };
};
#endif // LETTUCE_UI_PRIMITIVES_HPP