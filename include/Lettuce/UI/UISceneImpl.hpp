/*
Created by @PieroCastillo on 2026-08-13
*/
#ifndef LETTUCE_UI_UI_SCENE_IMPL_HPP
#define LETTUCE_UI_UI_SCENE_IMPL_HPP

// standard headers
#include <any>
#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory_resource>
#include <vector>

// project headers
#include "../Core/api.hpp"
#include "../Foundations/api.hpp"
#include "../Quimera/api.hpp"
#include "./api.hpp"

namespace Lettuce::UI
{
    struct UISceneImpl
    {
        std::array<uint8_t, 65536> m_initBuffer;
        std::pmr::monotonic_buffer_resource m_allocator;
        Surface* m_surface;
        std::vector<Controls::Primitives::Control*> m_tempQueue;
        std::vector<ControlInstance> m_instances;

        explicit UISceneImpl() : m_allocator(m_initBuffer.data(), m_initBuffer.size(), std::pmr::new_delete_resource())
        {
        }

        void Create(const UISceneDesc&);
        void Destroy();
    };
};
#endif // LETTUCE_UI_UI_SCENE_IMPL_HPP