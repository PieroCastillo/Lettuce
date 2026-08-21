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

namespace Lettuce::UI
{
    struct MouseButtonPressedEventArgs
    {
        uint32_t x, y;
    };

    enum class VerticalAlignment
    {
        Top = 1 << 0,
        VCenter = 1 << 1,
        Bottom = 1 << 2,
    };

    enum class HorizontalAlignment
    {
        Left = 1 << 3,
        HCenter = 1 << 4,
        Right = 1 << 5,
    };

    struct Style
    {

    };

    struct Label
    {
        std::string name;
        std::string content;
        Style style;
    };

    struct Button
    {
        std::string name;
        std::string content;
        std::function<void(std::any, bool)> command;
        std::function<void(MouseButtonPressedEventArgs)> onMousePressed;
        Style style;
    };

    struct UIView
    {

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

    struct ControlTag {};
    using Control = Handle<ControlTag>;
    constexpr auto InvalidControl = Control{ std::numeric_limits<uint32_t>::max(), 0 };
};
#endif // LETTUCE_UI_API_HPP