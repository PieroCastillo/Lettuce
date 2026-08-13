/*
Created by @PieroCastillo on 2026-08-12
*/
#ifndef EDITOR_EDITOR_VIEWPORT_HPP
#define EDITOR_EDITOR_VIEWPORT_HPP

#include <chrono>
#include <cstdint>
#include <span>
#include <string>

#include "Lettuce/Lettuce.hpp"
#include "common.hpp"
#include "SceneWorkspace.hpp"
#include "Window.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Foundations;

namespace Editor
{
    class EditorViewport
    {
        Device& device;
        DescriptorTable descriptorTable;
        TextureView pickTarget;

        std::unique_ptr<Debug::DebugPass> debugPass;
        GpuUniquePtr<uint32_t> pickInstance;
    public:
        EditorViewport(Window& window, Device& device, Swapchain swapchain);
        ~EditorViewport();

        void Resize(uint32_t width, uint32_t height);
        void Record(const RenderInfo& renderInfo, SceneWorkspace& scene);
        void RequestPick(CommandBuffer& cmd, uint32_t x, uint32_t y);

        auto GetPickTarget() const { return pickTarget; }

        auto GetPickResult() const -> PickResult
        {
            uint32_t value = *pickInstance;

            if (value == 0)
                return { PickedInstanceType::None, 0 };
            else if ((value & (1 << 31)) != 0)
                return { PickedInstanceType::UI2D, (value & ((1U << 31) - 1)) - 1 };
            else
                return { PickedInstanceType::Scene3D, value - 1 };
        }
    };
};
#endif // EDITOR_EDITOR_VIEWPORT_HPP