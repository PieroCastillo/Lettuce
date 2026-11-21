/*
Created by @PieroCastillo on 2026-05-17
*/
#ifndef LETTUCE_RENDERING_SCENE_VIEW_HPP
#define LETTUCE_RENDERING_SCENE_VIEW_HPP

#include "../Core/api.hpp"
#include "../Foundations/api.hpp"
#include "types.hpp"

using namespace Lettuce::Foundations;

namespace Lettuce::Rendering
{
    struct ShapesViewDesc
    {
        Device& device;
        uint32_t maxInstanceCount;
    };

    class ShapesView
    {
    private:
        Device* m_device = nullptr;
        CommandAllocator m_cmdAlloc;

        //  streams

        // data over vertices
    public:
        ShapesView() noexcept = default;
        explicit ShapesView(const ShapesViewDesc& desc);

        ~ShapesView();

        ShapesView(const ShapesView&) = delete;
        ShapesView& operator=(const ShapesView&) = delete;

        ShapesView(ShapesView&&) noexcept;
        ShapesView& operator=(ShapesView&&) noexcept;

        void Create(const ShapesViewDesc&);
        void Destroy();
    };
};
#endif // LETTUCE_RENDERING_SCENE_VIEW_HPP