/*
Created by @PieroCastillo on 2026-08-11
*/
#ifndef EDITOR_VIEWPORT_HPP
#define EDITOR_VIEWPORT_HPP

#include <chrono>
#include <cstdint>
#include <span>
#include <string>

#include "Lettuce/Lettuce.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Foundations;

namespace Editor
{
    class SceneEditor
    {
        std::unique_ptr<SceneView> scene;
        GpuUniquePtr<SceneViewData> sceneViewData;
        Lettuce::Utils::Camera3D camera;
    public:
        SceneEditor(Device& device);
        ~SceneEditor();

        void Update(const Lettuce::Utils::FrameTimer&, const InputSystem&);

        SceneView& GetView() { return *scene; }
        GpuSpan<SceneViewData> GetViewData() { return GpuSpan(sceneViewData); }
    };
};
#endif // EDITOR_VIEWPORT_HPP