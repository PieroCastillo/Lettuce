/*
Created by @PieroCastillo on 2026-08-11
*/
#ifndef EDITOR_SCENE_WORKSPACE_HPP
#define EDITOR_SCENE_WORKSPACE_HPP

#include <chrono>
#include <cstdint>
#include <span>
#include <string>
#include <type_traits>

#include "Lettuce/Lettuce.hpp"
#include "common.hpp"
#include "Input.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Foundations;

namespace Editor
{
    class SceneWorkspace
    {
        std::unique_ptr<SceneView> scene;
        GpuUniquePtr<SceneViewData> sceneViewData;
        Lettuce::Utils::Camera3D camera;
    public:
        SceneWorkspace(Device& device);
        ~SceneWorkspace();

        void Update(const Lettuce::Utils::FrameTimer&, const InputSystem&, const PickResult&);

        SceneView& GetView() { return *scene; }
        GpuSpan<SceneViewData> GetViewData() { return GpuSpan(sceneViewData); }
    };
};
#endif // EDITOR_SCENE_WORKSPACE_HPP