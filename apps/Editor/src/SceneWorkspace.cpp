#include <filesystem>
#include <memory>

#include "Lettuce/Lettuce.hpp"
#include "common.hpp"
#include "Renderer.hpp"
#include "Input.hpp"
#include "SceneWorkspace.hpp"

using namespace Editor;

SceneWorkspace::SceneWorkspace(Device& device)
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

    sceneViewData->viewProj = camera.Update({ false, false, false, false, 0.5f });
}

SceneWorkspace::~SceneWorkspace()
{
    sceneViewData.reset();
    scene.reset();
}

void SceneWorkspace::Update(const Lettuce::Utils::FrameTimer& timer, const InputState& state, const PickResult& pick)
{
    double dt = timer.GetDeltaTime();

    if (state.mouseLeftPressed)
    {
        auto delta = state.mouseDelta;
        camera.Rotate({ delta.x , delta.y });
    }

    if (state.activeKeys[InputKey::Ctrl])
    {
        sceneViewData->viewProj = camera.Update({
            state.activeKeys[InputKey::W],  state.activeKeys[InputKey::A],
            state.activeKeys[InputKey::S],  state.activeKeys[InputKey::D],
            static_cast<float>(dt)});
        return;
    }

    if (pick.type != PickedInstanceType::Scene3D)
        return;
        
    auto instanceIdx = pick.instanceID;
    auto direction = glm::vec<3, double>{};

    /*
    W     this seems like this: |
    S D                         . __
    */
    if (state.activeKeys[InputKey::W])
        direction.y = 1;
    if (state.activeKeys[InputKey::S])
        direction.z = 1;
    if (state.activeKeys[InputKey::D])
        direction.x = 1;

    if (state.activeKeys[InputKey::X])
        direction *= -1;

    auto speed = 0.5 * direction;
    auto dr = (float3)(speed * dt);

    auto transform = scene->GetInstanceTable()[instanceIdx].localTransform;
    scene->GetInstanceTable()[instanceIdx].localTransform = glm::translate(float4x4(1.0f), float3(dr)) * transform;
}