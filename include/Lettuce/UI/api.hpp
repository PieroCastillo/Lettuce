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
    struct UISceneDesc
    {
        Surface& surface;
    };

    struct UISceneImpl;
    class UIScene
    {
    private:
        UISceneImpl* impl = nullptr;
        auto alloc(size_t Tsize, size_t Talignment) -> void*;
    public:
        UIScene() = default;
        explicit UIScene(const UISceneDesc&);
        ~UIScene();

        UIScene(const UIScene&) = delete;
        UIScene& operator=(const UIScene&) = delete;

        UIScene(UIScene&&) noexcept;
        UIScene& operator=(UIScene&&) noexcept;

        template<Controls::Primitives::ControlDerivate T>
        auto Create() -> T&
        {
            auto* rawMem = alloc(sizeof(T), alignof(T));
            auto* controlPtr = ::new (rawMem) T();
            return *controlPtr;
        }

        template<Controls::Primitives::ControlDerivate T, typename... Args>
        T* Create(Args&&... args);

        void Build(std::weak_ptr<Controls::Primitives::Control> visualRoot);
        void Update(const InputState&);
        void Record(CommandBuffer&);
    };
};
#endif // LETTUCE_UI_API_HPP