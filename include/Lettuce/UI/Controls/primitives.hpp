/*
Created by @PieroCastillo on 2026-08-29
*/
#ifndef LETTUCE_UI_PRIMITIVES_HPP
#define LETTUCE_UI_PRIMITIVES_HPP

// standard headers
#include <any>
#include <atomic>
#include <concepts>
#include <cstdint>
#include <limits>
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

        virtual ~Control() {};
        virtual auto Children() const -> std::span<const std::reference_wrapper<Control>> { return {}; }
        virtual void Build(Surface&, ControlInstance&) {};
        virtual void Reset(Surface&, ControlInstance&) {};
        virtual auto Layout(ControlInstance&, float4 available) -> float4 { return {}; };
        virtual void Update(ControlInstance&, const InputState&) {};
        virtual void Render(ControlInstance&, SurfaceCommandBuffer&) {};
    };

    template<typename T>
    concept ControlDerivate = std::derived_from<T, Control>;

    using ControlRef = std::reference_wrapper<Control>;
    using ControlPtr = std::unique_ptr<Control>;

    struct ContentControl : public Control
    {
        std::any Content;
        std::move_only_function<ControlPtr(const std::any&)> GetControl;
    };

    struct ItemControl : public Control
    {
        ObservableVector<std::any> Items;
        std::move_only_function<ControlPtr(const std::any&)> ItemTemplate;
    };

    struct SelectingItemControl : public Control
    {
        uint32_t selectedIndex = std::numeric_limits<uint32_t>::max();
        std::move_only_function<void(uint32_t)> OnSelection;
        virtual void Select(uint32_t index);
    };

    struct ViewControl : public Control
    {
        std::vector<ControlRef> children;
    };
};
#endif // LETTUCE_UI_PRIMITIVES_HPP