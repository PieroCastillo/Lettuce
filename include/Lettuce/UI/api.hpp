/*
Created by @PieroCastillo on 2026-08-13
*/
#ifndef LETTUCE_UI_API_HPP
#define LETTUCE_UI_API_HPP

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
#include "types.hpp"
#include "Controls/primitives.hpp"
#include "Controls/controls.hpp"

namespace Lettuce::UI
{
    struct UIView
    {
        std::unique_ptr<Controls::Primitives::Control> child;
    };

    struct UISceneImpl;
    class UIScene
    {
    private:
        UISceneImpl* impl = nullptr;
    public:
        UIScene() = default;
        explicit UIScene(std::move_only_function<UIView(void)> builder);
        ~UIScene();

        void Build(std::move_only_function<UIView(void)> builder);
        void Update();

        void Record(CommandBuffer&);
    };
};
#endif // LETTUCE_UI_API_HPP