#include <filesystem>
#include <memory>

#include "Lettuce/Lettuce.hpp"
#include "Renderer.hpp"
#include "Input.hpp"
#include "SceneEditor.hpp"

using namespace Editor;

SceneEditor::SceneEditor(Device& device)
{
    std::filesystem::path modelPath = "../../../../external/models/DragonAttenuation.glb";

    auto srcs = std::vector<GeometrySource>();
    srcs.push_back(Lettuce::Utils::AssetLoader::LoadGtlfAsGeometry(&device, modelPath.string()));

    SceneViewDesc sceneDesc = {
        .device = device,
        .sources = srcs,
        .maxInstanceCount = 20,
    };
    scene = std::make_unique<SceneView>(sceneDesc);

    sceneViewData = GpuUniquePtr<SceneViewData>(device);
    sceneViewData->instanceCount = scene->GetInstanceTable().size();

    Lettuce::Utils::Camera3DDesc camDesc;
    camera = Lettuce::Utils::Camera3D(camDesc);
}

SceneEditor::~SceneEditor()
{
    sceneViewData.reset();
    scene.reset();
}

void SceneEditor::Update(const Lettuce::Utils::FrameTimer& timer, const InputSystem& input)
{
    double dt = timer.GetDeltaTime();

    auto state = input.GetState();
    if (state.mouseLeftPressed)
    {
        auto delta = state.mouseDelta;
        camera.Rotate({ delta.x , delta.y });
    }

    sceneViewData->viewProj = camera.Update({ state.wKeyPressed,state.aKeyPressed, state.sKeyPressed, state.dKeyPressed, static_cast<float>(dt) });
}