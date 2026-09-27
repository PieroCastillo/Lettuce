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
#include <memory>
#include <vector>

// project headers
#include "../../Core/api.hpp"
#include "../../Foundations/api.hpp"
#include "../../Quimera/api.hpp"
#include "../input.hpp"
#include "../mvvm.hpp"

using namespace Lettuce::Quimera;

namespace Lettuce::UI::Controls::Primitives
{   
    struct Control
    {
        std::string name;
        std::shared_ptr<Style> style;
        VerticalAlignment verticalAlignment;
        HorizontalAlignment horizontalAlignment;
/*
        virtual auto Build() -> ControlInstance = 0;
        virtual void Reset() = 0;
        virtual auto Layout(float4) -> float4 = 0;
        virtual void Update(ControlInstance&, const InputState&) = 0;
        virtual void Render(ControlInstance&, SurfaceCommandBuffer&) = 0;
*/
    };

    struct ContentControl : public Control
    {
        std::any Content;
        std::function<std::unique_ptr<Control>(std::any)> GetControl;
    };

    struct ItemControl : public Control
    {
        ObservableVector<std::any> Items;
        std::move_only_function<std::unique_ptr<Control>(std::any)> ItemTemplate;
    };

    struct SelectingItemControl : public Control
    {
        std::function<void(std::unique_ptr<Control>&)> onSelection;
    };

    struct ViewControl : public Control
    {
        std::vector<std::unique_ptr<Control>> Children;
    };
};
#endif // LETTUCE_UI_PRIMITIVES_HPP